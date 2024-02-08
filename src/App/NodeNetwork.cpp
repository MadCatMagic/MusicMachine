#include "App/NodeNetwork.h"
#include "App/Canvas.h"
#include "imgui.h"
#include <unordered_map>
#include "BBox.h"
#include "Random.h"

#include "JSON/JSON.h"

#include "Engine/Console.h"
#include <deque>
#include <sstream>

NodeNetwork* NodeNetwork::context = nullptr;

void NodeNetwork::ExecuteCommand(std::vector<std::string> args)
{
	if (context == nullptr)
		return;

	Console::Log("Executing NodeNetwork...");
	if (context->Execute())
		Console::Log("success=true");
	else
		Console::Log("success=false");
	for (Node* n : context->nodeDependencyInfoPersistent->endpoints)
		Console::Log("Result: " + n->Result());
}

NodeNetwork::NodeNetwork()
{
	if (context == nullptr)
		Console::AddCommand(ExecuteCommand, "exec");
	context = this;
}

NodeNetwork::NodeNetwork(const std::string& nnFilePath)
{
	JSONConverter conv;
	auto res = conv.DecodeFile(nnFilePath);
	if (!res.second)
	{
		Console::LogWarn("Failed to open or decode file <" + nnFilePath + ">!");
		return;
	}

	// first pass creates the nodes themselves and reassigns their ids
	auto& arr = res.first["nodes"].arr;
	for (JSONType& t : arr)
	{
		Node* node = CreateRawNode(t.obj["name"].s);
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
		node->IO();
		node->UpdateDimensions();
		nodes.push_back(node);
		nodeIDMap[node->id_s()] = node;
	}

	// second pass calls the load functions on the nodes to load in node-specific data
	// and also to create all the inputs and outputs required.
	for (size_t i = 0; i < nodes.size(); i++)
		nodes[i]->LoadData(arr[i]);
}

NodeNetwork::~NodeNetwork()
{
	for (Node* n : nodes)
		delete n;
	nodes.clear();
}

void NodeNetwork::Draw(DrawList* drawList, Canvas* canvas, std::vector<Node*>& selected, const bbox2& screen)
{
	drawList->dl->ChannelsSplit(2 * (int)nodes.size());
	int currentChannel = -1;
	currentList = drawList;
	currentCanvas = canvas;
	for (Node* node : nodes)
	{
		currentChannel += 2;
		drawList->dl->ChannelsSetCurrent(currentChannel);

		// make sure to add the correct node connections to the buffer
		node->ResetTouchedStatus();
		node->IO();
		node->CheckTouchedStatus();
		// automatically sets the size
		bool dontCullNode = screen.overlaps(node->getBounds());
		node->Draw(this, !dontCullNode);

		if (dontCullNode)
		{
			// draw node body
			bbox2 nodeBounds = node->getBounds();
			v2 topLeft = nodeBounds.a;
			v2 bottomRight = nodeBounds.b;
			float rounding = node->mini ? node->headerSize() : NODE_ROUNDING;

			bool isSelected = std::find(selected.begin(), selected.end(), node) != selected.end();
			bool isSelectedTop = selected.size() > 0 && selected[selected.size() - 1] == node;

			DrawColour outline = isSelected ? DrawColour::Node_SelectedOutline : DrawColour::Node_BGOutline;
			outline = isSelectedTop ? DrawColour::Node_TopSelectedOutline : outline;
			DrawColour fill = isSelected ? DrawColour::Node_SelectedFill : DrawColour::Node_BGFill;

			// rounded to 4 pixels - a single grid tile.
			currentChannel--;
			drawList->dl->ChannelsSetCurrent(currentChannel);
			drawList->RectFilled(
				nodeBounds.a - 1.0f,
				nodeBounds.b + 1.0f,
				outline,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
			drawList->RectFilled(
				topLeft,
				bottomRight,
				fill,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
		}
	}
	drawList->dl->ChannelsMerge();

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

	// draw connections on top of all the nodes
	for (const ConnectionToDraw& connection : connectionsToDraw)
		if (nodeDependencyInfoPersistent->problemConnectionExists && (
			connection.from == nodeDependencyInfoPersistent->problemConnection.first &&
			connection.to == nodeDependencyInfoPersistent->problemConnection.second ||
			connection.to == nodeDependencyInfoPersistent->problemConnection.first &&
			connection.from == nodeDependencyInfoPersistent->problemConnection.second))
			drawList->BezierCubic(connection.a, connection.b, connection.c, connection.d, DrawColour::Node_ConnectorInvalid, connection.thickness);
		else
			drawList->BezierCubic(connection.a, connection.b, connection.c, connection.d, connection.col, connection.thickness);
	connectionsToDraw.clear();

	// draw debugging stuff
	if (drawDebugInformation)
	{
		const float k_r = 0.2f;
		const float k_a = 0.02f;

		std::vector<AbstractNode*>& absNodes = nodeDependencyInfoPersistent->nodes;
		srand(10);
		std::vector<v2> positions = std::vector<v2>(absNodes.size());
		for (size_t i = 0; i < absNodes.size(); i++)
			positions[i] = Random::randv2() * 50.0f - 25.0f;
		// pretty slow
		for (size_t iter = 0; iter < 20; iter++)
		{
			// repulsive force between nodes
			for (size_t i = 0; i < absNodes.size(); i++)
				for (size_t j = i + 1; j < absNodes.size(); j++)
				{
					const v2 diff = positions[j] - positions[i];
					const float dist = std::max(0.1f, diff.length());
					const v2 force = diff * (k_r / dist);
					positions[i] -= force;
					positions[j] += force;
				}

			// attractive force along edges
			// only consider node inputs so not to repeat anything
			for (size_t i = 0; i < absNodes.size(); i++)
				for (size_t j : absNodes[i]->inputs)
				{
					const v2 diff = positions[j] - positions[i];
					const float dist = std::max(0.1f, diff.length());
					const v2 force = diff * (k_a / dist);
					positions[i] -= force;
					positions[j] += force;
				}
		}

		// actually draw the damn thing
		// again only care about inputs
		drawList->convertPosition = false;
		for (size_t i = 0; i < absNodes.size(); i++)
		{
			v2 origin = positions[i] + canvas->ScreenToCanvas(-100.0f);
			for (size_t j : absNodes[i]->inputs)
			{
				v2 endpoint = positions[j] + canvas->ScreenToCanvas(-100.0f);
				ImColor col = ImColor(1.0f, 0.0f, 1.0f);
				if (std::find(absNodes[j]->markedBy.begin(), absNodes[j]->markedBy.end(), i) != absNodes[j]->markedBy.end())
					col = ImColor(0.0f, 1.0f, 1.0f);
				if (
					nodeDependencyInfoPersistent->problemConnectionExists &&
					(j == nodeDependencyInfoPersistent->problemConnection.first && 
					i == nodeDependencyInfoPersistent->problemConnection.second ||
					i == nodeDependencyInfoPersistent->problemConnection.first &&
					j == nodeDependencyInfoPersistent->problemConnection.second)
				)
					col = ImColor(1.0f, 1.0f, 0.0f);
				drawList->Line(origin, endpoint, col, 2.0f);
			}
		}
		for (size_t i = 0; i < absNodes.size(); i++)
		{
			v2 origin = positions[i] + canvas->ScreenToCanvas(-100.0f);
			ImColor col = absNodes[i]->isEndpoint ? DrawColour::Node_IOBool : DrawColour::Text;
			drawList->CircleFilled(origin, 4.0f, col);
		}
		drawList->convertPosition = true;
	}
}

#include "App/NodeTypes.h"
Node* NodeNetwork::AddNodeFromName(const std::string& type, bool positionFromCursor)
{
	Node* n = CreateRawNode(type);

	if (n == nullptr)
		return nullptr;

	// random enough for now but unreliable
	n->NodeInit(this, (uint64_t)rand() << 16 ^ nodes.size());
	n->Init();
	n->UpdateDimensions();
	if (positionFromCursor)
		n->position = currentCanvas->CanvasToPosition(currentCanvas->ScreenToCanvas(ImGui::GetMousePos())) - n->size * 0.5f;
	nodes.push_back(n);
	nodeIDMap[n->id_s()] = n;
	return n;
}

Node* NodeNetwork::GetNodeAtPosition(const v2& pos, Node* currentSelection, size_t offset)
{
	if (currentSelection == nullptr)
	{
		for (Node* node : nodes)
			if (node->getBounds().containsLeniant(pos, 4.0f))
				return node;
	}
	else
	{
		size_t index = std::find(nodes.begin(), nodes.end(), currentSelection) - nodes.begin() + offset;
		for (size_t i = index; i < nodes.size() + index; i++)
		{
			Node* n = nodes[i % nodes.size()];
			if (n->getBounds().containsLeniant(pos, 4.0f))
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
		if (bb.overlaps(n->getBounds()))
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

void NodeNetwork::DrawInput(const v2& cursor, const Node::NodeInput& inp, float width)
{
	float sf = currentCanvas->GetSF().x;
	if (inp.target != nullptr && inp.source == nullptr)
	{
		// interaction for floats and integers
		if ((inp.type == Node::NodeType::Float || inp.type == Node::NodeType::Int))
		{
			float value;
			if (inp.type == Node::NodeType::Float)
				value = *(float*)inp.target;
			else
				value = (float)(*(int*)inp.target);
			// drawing the box representing the value
			v2 tl = cursor + v2(0.0f, -8.0f);
			float proportion = (value - inp.fmin) / (inp.fmax - inp.fmin);
			v2 br = cursor + v2(
				std::min(std::max(proportion, 0.0f), 1.0f) * width,
				8.0f
			);
			currentList->RectFilled(tl, br, DrawColour::Node_BGHeader);
			// drawing the text displaying the value
			std::ostringstream ss;
			ss.precision(3);
			if (inp.type == Node::NodeType::Float)
				ss << value;
			else
				ss << *(int*)inp.target;
			std::string convertedString = ss.str();
			v2 ftpos = cursor + v2(width - (convertedString.size() + 1) * 6.0f, -6.0f);
			currentList->Text(
				ftpos,
				DrawColour::TextFaded,
				convertedString.c_str()
			);
		}
		// interaction for bool inputs
		else if (inp.type == Node::NodeType::Bool)
		{
			v2 centre = cursor + v2(width - 10.0f, 0.0f);
			currentList->Circle(centre, 4.0f / sf, DrawColour::TextFaded, 1.0f / sf);
			if (*(bool*)inp.target)
				currentList->CircleFilled(centre, 2.0f / sf, DrawColour::TextFaded);
		}
	}
	v2 pos = cursor + v2(8.0f, -6.0f);
	currentList->Text(pos, DrawColour::Text, inp.name.c_str());
	// input circle thingy
	pos = cursor;
	DrawConnectionEndpoint(pos, GetCol(inp.type), true, inp.target == nullptr);
}

void NodeNetwork::DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out)
{
	v2 pos = cursor + v2(xOffset, 0.0f);
	float sf = currentCanvas->GetSF().x;
	// draw on right side of node
	DrawConnectionEndpoint(pos, GetCol(out.type), true, out.data == nullptr);
	// text
	pos = cursor + v2(xOffset - (out.name.size() + 1) * 6.0f, 0.0f) + v2(-8.0f, -6.0f);
	currentList->Text(pos, DrawColour::Text, out.name.c_str());
}

void NodeNetwork::DrawConnectionEndpoint(const v2& centre, DrawColour col, bool convertPosition, bool isNull)
{
	// draw it
	float sf = currentCanvas->GetSF().x;
	currentList->convertPosition = convertPosition;
	currentList->CircleFilled(centre, 4.0f / sf, DrawColour::Node_IO);
	currentList->CircleFilled(centre, 3.0f / sf, col);
	if (isNull)
		currentList->Circle(centre, 2.0f / sf, DrawColour::Node_IO, 1.0f / sf);
	currentList->convertPosition = true;
}

void NodeNetwork::DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset)
{
	v2 topLeft = cursor + 1.0f;
	v2 bottomRight = cursor + v2(width, height) - 1.0f;
	v2 textPos = v2(cursor.x + 8.0f, cursor.y + height * 0.5f - 6.0f);
	ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
	float rounding = mini ? height * 0.5f: NODE_ROUNDING;
	currentList->RectFilled(topLeft, bottomRight, DrawColour::Node_BGHeader, rounding / currentCanvas->GetSF().x, flags);
	currentList->Text(textPos, DrawColour::Text, name.c_str());

	// minimized triangle thing
	// position should be constant regardless of header size
	v2 triCentre = cursor + v2(miniTriOffset, height * 0.5f);
	if (mini)
		currentList->TriangleFilled(
			triCentre + v2(3.0f, 0.0f), 
			triCentre + v2(-3.0f, -3.0f), 
			triCentre + v2(-3.0f, 3.0f), 
			DrawColour::Text
		);
	else
		currentList->TriangleFilled(
			triCentre + v2(0.0f, 3.0f), 
			triCentre + v2(-3.0f, -3.0f), 
			triCentre + v2(3.0f, -3.0f), 
			DrawColour::Text
		);
}

void NodeNetwork::DrawConnection(const v2& target, const v2& origin, Node::NodeType type, Node* from, Node* to)
{
	size_t f = -1;
	size_t t = -1;
	if (from != nullptr && to != nullptr)
	{
		for (size_t i = 0; i < nodes.size(); i++)
		{
			if (nodes[i] == from)
				f = i;
			if (nodes[i] == to)
				t = i;
		}
	}
	float width = 12.0f + fabsf(target.x - origin.x) * 0.3f + fabsf(target.y - origin.y) * 0.1f;
	ConnectionToDraw c{
		origin,
		origin + v2(width, 0.0f),
		target - v2(width, 0.0f),
		target,
		GetCol(type),
		1.5f / currentCanvas->GetSF().x,
		f,
		t
	};
	connectionsToDraw.push_back(c);
}

void NodeNetwork::DrawContextMenu()
{
	if (ImGui::BeginMenu("Nodes"))
	{
		const std::string nodeNames[] {
			"Node",
			"MathsNode",
			"LongNode",
			"ConstNode"
		};
		for (const std::string& name : nodeNames)
			if (ImGui::MenuItem(name.c_str()))
				AddNodeFromName(name, true);
		ImGui::EndMenu();
	}

	ImGui::MenuItem("Debug Enable", nullptr, &drawDebugInformation);
}

DrawColour NodeNetwork::GetCol(Node::NodeType type)
{
	if (type == Node::NodeType::Bool)
		return DrawColour::Node_IOBool;
	else if (type == Node::NodeType::Float)
		return DrawColour::Node_IOFloat;
	else if (type == Node::NodeType::Int)
		return DrawColour::Node_IOInt;
	return DrawColour::Canvas_BG;
}

Node* NodeNetwork::GetNodeFromID(const std::string& id)
{
	if (nodeIDMap.find(id) != nodeIDMap.end())
		return nodeIDMap[id];
	return nullptr;
}

Node* NodeNetwork::CreateRawNode(const std::string& type)
{
	if (type == "Node")
		return new Node();
	if (type == "MathsNode")
		return new MathsNode();
	if (type == "LongNode")
		return new LongNode();
	if (type == "ConstNode")
		return new ConstNode();
	return nullptr;
}

bool NodeNetwork::Execute()
{
	// dont execute if there is a dependency problem
	if (nodeDependencyInfoPersistent->problemConnectionExists)
		return false;

	// backpropagate in a sensible manner
	for (Node* n : nodes)
		n->hasBeenExecuted = false;
	for (Node* e : nodeDependencyInfoPersistent->endpoints)
		e->Execute();
	return true;
}

void NodeNetwork::SaveNetworkToFile(const std::string& nnFilePath)
{
	JSONConverter conv;
	JSONType nodeData = JSONType(JSONType::Array);

	for (Node* n : nodes)
		nodeData.arr.push_back(n->SaveData());

	conv.WriteFile(nnFilePath, JSONType({
		{ "nodes", nodeData },
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

	std::deque<std::pair<size_t, bool>> queue;

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
			queue.push_back(std::make_pair(i, false));
			
		absNodes.push_back(n);

		nodeMap[nodes[i]] = i;
	}

	// now fill out all of the nodes's connections with each other
	for (size_t i = 0; i < nodes.size(); i++)
		for (Node::NodeInput& k : nodes[i]->inputs)
			if (k.source != nullptr && std::find(absNodes[i]->inputs.begin(), absNodes[i]->inputs.end(), nodeMap[k.source]) == absNodes[i]->inputs.end())
			{
				absNodes[i]->inputs.push_back(nodeMap[k.source]);
				absNodes[nodeMap[k.source]]->outputs.push_back(i);
			}

	// backpropagate, marking nodes as you go, using the endpoints 'queue'

	NodeDependencyInformation* nodeDependencyInformation = new NodeDependencyInformation();
	nodeDependencyInformation->nodes = absNodes;
	nodeDependencyInformation->endpoints = endpoints;

	while (queue.size() > 0)
	{
		auto& pair = queue.front();
		size_t currentI = pair.first;
		AbstractNode* current = absNodes[currentI];
		queue.pop_front();

		for (size_t i = 0; i < current->inputs.size(); i++)
		{
			size_t inputI = current->inputs[i];
			AbstractNode* input = absNodes[inputI];
			// should not need to check whether outputIndex is actually valid, should always be
			if (std::find(input->markedBy.begin(), input->markedBy.end(), currentI) != input->markedBy.end())
			{
				if (pair.second)
				{
					// means that the loop is the next connection down kinda
					size_t error = 0;
					for (size_t connection : input->outputs)
						if (std::find(input->markedBy.begin(), input->markedBy.end(), connection) == input->markedBy.end())
						{
							error = connection;
							break;
						}
					nodeDependencyInformation->problemConnectionExists = true;
					nodeDependencyInformation->problemConnection = std::make_pair(inputI, error);
					return nodeDependencyInformation;
				}
				// panic! there is some circular shit going on
				nodeDependencyInformation->problemConnectionExists = true;
				nodeDependencyInformation->problemConnection = std::make_pair(inputI, currentI);
				return nodeDependencyInformation;
			}
			input->markedBy.push_back(currentI);
			// only add the new node if it is full of outputs
			if (input->markedBy.size() == input->outputs.size())
				queue.push_back(std::make_pair(inputI, false));
			else if (queue.size() == 0)
			{
				// panic! impossible to activate node
				// have to find erroneous connection :sadpenisbee:
				size_t error = 0;
				for (size_t connection : input->outputs)
					if (std::find(input->markedBy.begin(), input->markedBy.end(), connection) == input->markedBy.end())
					{
						error = connection;
						break;
					}
				nodeDependencyInformation->problemConnectionExists = true;
				nodeDependencyInformation->problemConnection = std::make_pair(inputI, error);
				return nodeDependencyInformation;
			}
			else
				queue.push_back(std::make_pair(currentI, true));
		}
	}
	return nodeDependencyInformation;
}

NodeNetwork::NodeDependencyInformation::~NodeDependencyInformation()
{
	for (AbstractNode* n : nodes)
		delete n;
}
