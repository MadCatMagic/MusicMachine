#include "App/Nodes/NodeNetwork.h"
#include "App/Nodes/Canvas.h"
#include "App/Nodes/NodeFactory.h"
#include "App/Nodes/NodeTypes/NodeNetworkVariable.h"
#include "App/Nodes/NodeTypes/NodeNetworkNode.h"

#include "App/JSON.h"

#include "Engine/Console.h"
#include "BBox.h"
#include "Random.h"

#include "imgui.h"

#include <deque>
#include <sstream>
#include <unordered_map>

NodeNetwork::NodeNetwork()
{
}

NodeNetwork::NodeNetwork(const std::string& nnFilePath)
{
	JSONConverter conv;
	name = nnFilePath;
	auto res = conv.DecodeFile(nnFilePath);
	if (!res.second)
	{
		Console::LogWarn("Failed to open or decode file <" + nnFilePath + ">!");
		return;
	}

	// check if node should be set as root
	isRoot = res.first["root"].b;

	// first pass creates the nodes themselves and reassigns their ids
	auto& arr = res.first["nodes"].arr;
	for (JSONType& t : arr)
	{
		Node* node = AddNodeFromName(t.obj["name"].s, v2::zero, true);
		if (node == nullptr)
		{
			Console::LogErr("Unrecognised node name '" + t.obj["name"].s + "'.");
			continue;
		}
		std::string id = t.obj["id"].s;
		uint64_t idv = -1;
		for (size_t i = 0; i < id.size(); i++)
			if (id[i] == '_')
			{
				std::istringstream iss(id.substr(0, i));
				iss >> std::hex >> idv;
			}
		node->NodeInit(this, idv);
		node->Init();
		node->Load(t.obj["node"]);
		node->IO();
		node->renderer.UpdateDimensions();
		nodes.push_back(node);
		nodeIDMap[node->id_s()] = node;
	}

	// second pass calls the load functions on the nodes to load in node-specific data
	// and also to create all the inputs and outputs required.
	for (size_t i = 0; i < nodes.size(); i++)
		nodes[i]->LoadData(arr[i]);

	for (size_t i = 0; i < nodes.size(); i++)
	{
		nodes[i]->ResetTouchedStatus();
		nodes[i]->IO();
		nodes[i]->CheckTouchedStatus();
	}

	RecalculateDependencies();
	Update();
	Console::Log("Loaded NodeNetwork from file <" + nnFilePath + ">.");
}

NodeNetwork::~NodeNetwork()
{
	for (Node* n : nodes)
		delete n;
	nodes.clear();
}

Node* NodeNetwork::AddNodeFromName(const std::string& type, const v2& initPos, bool skipInitialisation)
{
	Node* n;
	if (type == "NodeNetworkVariable")
		n = new NodeNetworkVariable();
	else if (type == "NodeNetworkNode")
		n = new NodeNetworkNode();
	else
		n = GetNodeFactory().Build(type);

	if (n == nullptr)
		return nullptr;

	if (skipInitialisation)
		return n;

	// random enough for now but unreliable
	n->NodeInit(this, (uint64_t)rand() << 16 ^ nodes.size());
	n->Init();
	n->renderer.UpdateDimensions();
	n->position = initPos - n->renderer.size * 0.5f;
	nodes.push_back(n);
	nodeIDMap[n->id_s()] = n;
	return n;
}

Node* NodeNetwork::GetNodeAtPosition(const v2& pos, Node* currentSelection, size_t offset)
{
	if (currentSelection == nullptr)
	{
		for (Node* node : nodes)
			if (node->renderer.getBounds().containsLeniant(pos, 4.0f))
				return node;
	}
	else
	{
		size_t index = std::find(nodes.begin(), nodes.end(), currentSelection) - nodes.begin() + offset;
		for (size_t i = index; i < nodes.size() + index; i++)
		{
			Node* n = nodes[i % nodes.size()];
			if (n->renderer.getBounds().containsLeniant(pos, 4.0f))
				return n;
		}
	}
	return nullptr;
}

std::vector<Node*> NodeNetwork::FindNodesInArea(const v2& p1, const v2& p2)
{
	std::vector<Node*> v = std::vector<Node*>();
	bbox2 bb = bbox2(p1, p2);
	for (Node* n : nodes)
		if (bb.overlaps(n->renderer.getBounds()))
			v.push_back(n);
	return v;
}

void NodeNetwork::PushNodeToTop(Node* node)
{
	auto index = std::find(nodes.begin(), nodes.end(), node);
	// dont do anything if it is either not in the network, or the last element anyway
	if (index < nodes.end() - 1)
	{
		nodes.erase(index);
		nodes.push_back(node);
		recalculateDependencies = true;
	}
}

void NodeNetwork::TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed)
{
	for (Node* n : nodes)
		if (n != origin && n->TryConnect(origin, originName, pos, connectionReversed))
			recalculateDependencies = true;
			return;
}

void NodeNetwork::DeleteNode(Node* node)
{
	for (Node* n : nodes)
		for (size_t i = 0; i < n->inputs.size(); i++)
			if (n->inputs[i].source != nullptr && n->inputs[i].source == node)
				n->Disconnect(i);

	nodes.erase(std::find(nodes.begin(), nodes.end(), node));
	nodeIDMap.erase(node->id_s());
	delete node;
	recalculateDependencies = true;
}

std::pair<bool, bool> NodeNetwork::DrawContextMenu(const v2& contextMenuClickPos)
{
	if (ImGui::BeginMenu("Nodes"))
	{
		for (auto& pair : GetNodeFactory().Names())
		{
			if (!isRoot && pair.second == "Analysis")
				continue;
			if (ImGui::MenuItem(pair.second.c_str()))
				AddNodeFromName(pair.first, contextMenuClickPos);
		}
		ImGui::EndMenu();
	}

	bool beginNetworkNodePrompt = false;
	if (ImGui::MenuItem("New Network Node"))
		beginNetworkNodePrompt = true;

	bool beginNetworkVariablePrompt = false;
	if (!isRoot)
		if (ImGui::MenuItem("New Network Variable"))
			beginNetworkVariablePrompt = true;

	ImGui::MenuItem("Debug Enable", nullptr, &drawDebugInformation);

	return std::make_pair(beginNetworkNodePrompt, beginNetworkVariablePrompt);
}

Node* NodeNetwork::GetNodeFromID(const std::string& id)
{
	if (nodeIDMap.find(id) != nodeIDMap.end())
		return nodeIDMap[id];
	return nullptr;
}

bool NodeNetwork::Execute(bool setAudioStream, int ownedID)
{
	// help
	if (nodeDependencyInfoPersistent == nullptr)
		return false;

	// dont execute if there is a dependency problem
	if (nodeDependencyInfoPersistent->problemConnectionExists)
		return false;

	if (!Arranger::instance->playing && setAudioStream)
	{
		auto emptyVec = std::vector<v2>(AudioChannel::bufferSize, v2());
		audioStream->SetData(emptyVec);
		return true;
	}

	// backpropagate in a sensible manner
	for (Node* n : nodes)
		n->hasBeenExecuted = false;
	if (nodeDependencyInfoPersistent->endpoints.size() > 1)
		Console::LogWarn("Multiple endpoints exist, panic!");
	for (Node* e : nodeDependencyInfoPersistent->endpoints)
	{
		e->Execute(ownedID);
		if (setAudioStream && e->name == "AudioOutputNode")
			audioStream->SetData(e->Result()->data);
	}
	return true;
}

void NodeNetwork::Update()
{
	// need this here so we know if to draw any invalid connections
	if (recalculateDependencies)
	{
		if (nodeDependencyInfoPersistent != nullptr)
			delete nodeDependencyInfoPersistent;

		nodeDependencyInfoPersistent = CheckForCircularDependency();
		recalculateDependencies = false;

		if (drawDebugInformation)
			Console::Log("DEBUG: recalculated node network dependency graph");
	}
}

void NodeNetwork::SaveNetworkToFile(const std::string& nnFilePath)
{
	name = nnFilePath;

	JSONConverter conv;
	JSONType nodeData = JSONType(JSONType::Array);

	for (Node* n : nodes)
		nodeData.arr.push_back(n->SaveData());

	conv.WriteFile(nnFilePath, JSONType({
		{ "nodes", nodeData },
		{ "root", isRoot },
		{ "dummy", false }
	}));

	Console::Log("Saved current node network data to file <" + nnFilePath + ">.");
}

NodeNetwork::NodeDependencyInformation* NodeNetwork::CheckForCircularDependency()
{
	// assume nodes with no endpoints are start points of backpropagation
	std::vector<Node*> endpoints;
	std::vector<AbstractNode*> absNodes;
	// lookup table for increased speed
	std::unordered_map<Node*, size_t> nodeMap;

	std::vector<size_t> endpointIDs;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		AbstractNode* n = new AbstractNode();
		n->id = (unsigned int)i;
		if (nodes[i]->outputs.size() == 0)
		{
			endpoints.push_back(nodes[i]);
			n->isEndpoint = true;
		}
		// fuckery to get it to detect problems properly
		bool isUnusedEndpoint = true;
		for (auto& aaa : nodes[i]->outputs)
			if (aaa.connections > 0)
			{
				isUnusedEndpoint = false;
				break;
			}
		if (isUnusedEndpoint)
			endpointIDs.push_back(i);
			
		absNodes.push_back(n);

		nodeMap[nodes[i]] = i;
	}

	// now fill out all of the nodes's connections with each other
	for (size_t i = 0; i < nodes.size(); i++)
		for (Node::NodeInput& k : nodes[i]->inputs)
			if (k.source != nullptr && std::find(absNodes[i]->inputs.begin(), absNodes[i]->inputs.end(), nodeMap[k.source]) == absNodes[i]->inputs.end())
				absNodes[i]->inputs.push_back(nodeMap[k.source]);

	NodeDependencyInformation* nodeDependencyInformation = new NodeDependencyInformation();
	nodeDependencyInformation->nodes = absNodes;
	nodeDependencyInformation->endpoints = endpoints;
	
	// use TarjanSCC algorithm on all endpoints, making sure to clean the absNodes pile each time
	for (size_t endpoint : endpointIDs)
	{
		std::vector<size_t> stack;
		auto result = TarjanSCCSearch(endpoint, stack, absNodes);
		if (result.first)
		{
			nodeDependencyInformation->problemConnectionExists = true;
			nodeDependencyInformation->problemConnection = result.second;
			return nodeDependencyInformation;
		}
		for (AbstractNode* n : absNodes)
			n->onStack = false;
	}
	
	return nodeDependencyInformation;
}

std::pair<bool, std::pair<size_t, size_t>> NodeNetwork::TarjanSCCSearch(size_t node, std::vector<size_t>& stack, std::vector<AbstractNode*>& nodes)
{
	nodes[node]->onStack = true;
	stack.push_back(node);

	for (size_t c : nodes[node]->inputs)
	{
		if (nodes[c]->onStack)
			return { true, { node, c } };

		auto result = TarjanSCCSearch(c, stack, nodes);

		if (result.first)
			return { true, result.second };
	}

	nodes[node]->onStack = false;
	stack.pop_back();

	return { false, {} };
}

NodeNetwork::NodeDependencyInformation::~NodeDependencyInformation()
{
	for (AbstractNode* n : nodes)
		delete n;
}
