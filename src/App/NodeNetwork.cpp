#include "App/NodeNetwork.h"
#include "App/Canvas.h"
#include "imgui.h"
#include <unordered_map>
#include "BBox.h"
#include "Random.h"

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
	InitColours();
}

NodeNetwork::~NodeNetwork()
{
	for (Node* n : nodes)
		delete n;
	nodes.clear();
}

void NodeNetwork::Draw(ImDrawList* drawList, Canvas* canvas, std::vector<Node*>& selected, const bbox2& screen)
{
	drawList->ChannelsSplit(2 * (int)nodes.size());
	int currentChannel = -1;
	currentList = drawList;
	currentCanvas = canvas;
	for (Node* node : nodes)
	{
		currentChannel += 2;
		drawList->ChannelsSetCurrent(currentChannel);

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
			v2 topLeft = canvas->ptcts(nodeBounds.a);
			v2 bottomRight = canvas->ptcts(nodeBounds.b);
			v2 tlO = canvas->ptcts(nodeBounds.a - 1.0f);
			v2 brO = canvas->ptcts(nodeBounds.b + 1.0f);
			float rounding = node->mini ? node->headerSize() : NODE_ROUNDING;

			bool isSelected = std::find(selected.begin(), selected.end(), node) != selected.end();
			bool isSelectedTop = selected.size() > 0 && selected[selected.size() - 1] == node;

			ImColor outline = GetCol(isSelected ? (NodeCol::SelectedOutline) : NodeCol::BGOutline);
			outline = isSelectedTop ? GetCol(NodeCol::TopSelectedOutline) : outline;
			ImColor fill = GetCol(isSelected ? NodeCol::SelectedFill : NodeCol::BGFill);

			// rounded to 4 pixels - a single grid tile.
			currentChannel--;
			drawList->ChannelsSetCurrent(currentChannel);
			drawList->AddRectFilled(
				tlO.ImGui(),
				brO.ImGui(),
				outline,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
			drawList->AddRectFilled(
				topLeft.ImGui(),
				bottomRight.ImGui(),
				fill,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
		}
	}
	drawList->ChannelsMerge();

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
			drawList->AddBezierCubic(connection.a, connection.b, connection.c, connection.d, GetCol(NodeCol::ConnectorInvalid), connection.thickness);
		else
			drawList->AddBezierCubic(connection.a, connection.b, connection.c, connection.d, connection.col, connection.thickness);
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
				drawList->AddLine(origin.ImGui(), endpoint.ImGui(), col, 2.0f);
			}
		}
		for (size_t i = 0; i < absNodes.size(); i++)
		{
			v2 origin = positions[i] + canvas->ScreenToCanvas(-100.0f);
			ImColor col = absNodes[i]->isEndpoint ? GetCol(NodeCol::IOBool) : GetCol(NodeCol::Text);
			drawList->AddCircleFilled(origin.ImGui(), 4.0f, col);
		}
	}
}

#include "App/NodeTypes.h"
Node* NodeNetwork::AddNodeFromName(const std::string& type, bool positionFromCursor)
{
	Node* n = nullptr;

	if (type == "Node")
		n = new Node();

	if (type == "Maths")
		n = new MathsNode();

	if (type == "Long")
		n = new LongNode();

	if (type == "Const")
		n = new ConstNode();

	if (n == nullptr)
		return nullptr;

	n->NodeInit(this);
	n->Init();
	n->UpdateDimensions();
	if (positionFromCursor)
		n->position = currentCanvas->CanvasToPosition(currentCanvas->ScreenToCanvas(ImGui::GetMousePos())) - n->size * 0.5f;
	nodes.push_back(n);
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
	delete node;
	recalculateDependencies = true;
}

void NodeNetwork::DrawInput(const v2& cursor, const Node::NodeInput& inp, float width)
{
	ImColor colour = GetCol(inp.type);
	float sf = currentCanvas->GetSF().x;
	// interaction for floats
	if (inp.type == Node::NodeType::Float && inp.target != nullptr && inp.source == nullptr)
	{
		// drawing the box representing the value
		v2 tl = currentCanvas->ptcts(cursor + v2(0.0f, -6.0f));
		float proportion = (*(float*)inp.target - inp.fmin) / (inp.fmax - inp.fmin);
		v2 br = currentCanvas->ptcts(cursor + v2(
			std::min(std::max(proportion, 0.0f), 1.0f) * width, 
			6.0f
		));
		currentList->AddRectFilled(tl.ImGui(), br.ImGui(), GetCol(NodeCol::BGHeader));
		// drawing the text displaying the value
		std::ostringstream ss;
		ss.precision(3);
		ss << *(float*)inp.target;
		std::string convertedString = ss.str();
		v2 ftpos = currentCanvas->ptcts(cursor + v2(width - (convertedString.size() + 1) * 6.0f, -6.0f));
		currentList->AddText(
			ftpos.ImGui(), 
			GetCol(NodeCol::Text) - ImColor(0.0f, 0.0f, 0.0f, 0.4f),
			convertedString.c_str()
		);
	}
	v2 pos = currentCanvas->ptcts(cursor + v2(8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), inp.name.c_str());
	// input circle thingy
	pos = currentCanvas->ptcts(cursor);
	DrawConnectionEndpoint(pos, colour, false, inp.target == nullptr);
}

void NodeNetwork::DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out)
{
	ImColor colour = GetCol(out.type);
	v2 pos = currentCanvas->ptcts(cursor + v2(xOffset, 0.0f));
	float sf = currentCanvas->GetSF().x;
	// draw on right side of node
	DrawConnectionEndpoint(pos, colour, false, out.data == nullptr);
	// text
	pos = currentCanvas->ptcts(cursor + v2(xOffset - (out.name.size() + 1) * 6.0f, 0.0f) + v2(-8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), out.name.c_str());
}

void NodeNetwork::DrawConnectionEndpoint(const v2& centre, const ImColor& color, bool convertPosition, bool isNull)
{
	// draw it
	float sf = currentCanvas->GetSF().x;
	if (convertPosition)
	{
		ImVec2 c = currentCanvas->ptcts(centre).ImGui();
		currentList->AddCircleFilled(c, 4.0f / sf, GetCol(NodeCol::IO));
		currentList->AddCircleFilled(c, 3.0f / sf, color);
		if (isNull)
			currentList->AddCircle(c, 2.0f / sf, GetCol(NodeCol::IO), 0, 1.0f / sf);
	}
	else
	{
		currentList->AddCircleFilled(centre.ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
		currentList->AddCircleFilled(centre.ImGui(), 3.0f / sf, color);
		if (isNull)
			currentList->AddCircle(centre.ImGui(), 2.0f / sf, GetCol(NodeCol::IO), 0, 1.0f / sf);
	}
}

void NodeNetwork::DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset)
{
	v2 topLeft = currentCanvas->ptcts(cursor + 1.0f);
	v2 bottomRight = currentCanvas->ptcts(cursor + v2(width, height) - 1.0f);
	v2 textPos = currentCanvas->ptcts(v2(cursor.x + 8.0f, cursor.y + height * 0.5f - 6.0f));
	ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
	float rounding = mini ? height * 0.5f: NODE_ROUNDING;
	currentList->AddRectFilled(topLeft.ImGui(), bottomRight.ImGui(), GetCol(NodeCol::BGHeader), rounding / currentCanvas->GetSF().x, flags);
	currentList->AddText(textPos.ImGui(), GetCol(NodeCol::Text), name.c_str());

	// minimized triangle thing
	// position should be constant regardless of header size
	v2 triCentre = cursor + v2(miniTriOffset, height * 0.5f);
	if (mini)
	{
		v2 a = currentCanvas->ptcts(triCentre + v2(3.0f, 0.0f));
		v2 b = currentCanvas->ptcts(triCentre + v2(-3.0f, -3.0f));
		v2 c = currentCanvas->ptcts(triCentre + v2(-3.0f, 3.0f));
		currentList->AddTriangleFilled(a.ImGui(), b.ImGui(), c.ImGui(), GetCol(NodeCol::Text));
	}
	else
	{
		v2 b = currentCanvas->ptcts(triCentre + v2(0.0f, 3.0f));
		v2 a = currentCanvas->ptcts(triCentre + v2(-3.0f, -3.0f));
		v2 c = currentCanvas->ptcts(triCentre + v2(3.0f, -3.0f));
		currentList->AddTriangleFilled(a.ImGui(), b.ImGui(), c.ImGui(), GetCol(NodeCol::Text));
	}
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
		currentCanvas->ptcts(origin).ImGui(),
		currentCanvas->ptcts(origin + v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target - v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target).ImGui(),
		GetCol(type),
		1.5f / currentCanvas->GetSF().x,
		f,
		t
	};
	connectionsToDraw.push_back(c);
}

void NodeNetwork::DrawContextMenu()
{
	if (ImGui::BeginMenu("Colours"))
	{
		for (int i = 0; i < NUM_COLOURS; i++)
			ImGui::ColorEdit4(colours[i].name.c_str(), &colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Nodes"))
	{
		const std::string nodeNames[] {
			"Node",
			"Maths",
			"Long",
			"Const"
		};
		for (const std::string& name : nodeNames)
			if (ImGui::MenuItem(name.c_str()))
				AddNodeFromName(name, true);
		ImGui::EndMenu();
	}

	ImGui::MenuItem("Debug Enable", nullptr, &drawDebugInformation);
}

ImColor NodeNetwork::GetCol(NodeCol colour)
{
	return colours[(int)colour].col;
}

ImColor NodeNetwork::GetCol(Node::NodeType type)
{
	ImColor colour = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
	if (type == Node::NodeType::Bool)
		colour = GetCol(NodeCol::IOBool);
	else if (type == Node::NodeType::Float)
		colour = GetCol(NodeCol::IOFloat);
	return colour;
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

void NodeNetwork::InitColours()
{
	for (int i = 0; i < NUM_COLOURS; i++)
	{
		NodeColourData c;
		switch ((NodeCol)i)
		{
		case NodeCol::BGFill:
			c.name = "BGFill";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
			break;

		case NodeCol::BGOutline:
			c.name = "BGOutline";
			c.col = ImColor(0.0f, 0.0f, 0.4f, 0.8f);
			break;

		case NodeCol::BGHeader:
			c.name = "BGHeader";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			break;

		case NodeCol::IOBool:
			c.name = "IOBool";
			c.col = ImColor(1.0f, 0.2f, 0.6f);
			break;

		case NodeCol::IOFloat:
			c.name = "IOFloat";
			c.col = ImColor(0.4f, 0.6f, 0.9f);
			break;

		case NodeCol::IO:
			c.name = "IO";
			c.col = ImColor(0.1f, 0.1f, 0.1f);
			break;

		case NodeCol::IOSelected:
			c.name = "IOSelected";
			c.col = ImColor(0.6f, 0.6f, 0.6f);
			break;
		
		case NodeCol::Connector:
			c.name = "Connector";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			break;

		case NodeCol::ConnectorInvalid:
			c.name = "ConnectorInvalid";
			c.col = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
			break;

		case NodeCol::Text:
			c.name = "Text";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			break;

		case NodeCol::SelectedOutline:
			c.name = "SelectedOutline";
			c.col = ImColor(0.2f, 0.6f, 1.0f, 0.8f);
			break;
		case NodeCol::TopSelectedOutline:
			c.name = "TopSelectedOutline";
			c.col = ImColor(0.5f, 0.8f, 1.0f, 0.8f);
			break;
		case NodeCol::SelectedFill:
			c.name = "SelectedFill";
			c.col = ImColor(0.2f, 0.2f, 0.5f, 1.0f);
			break;
		case NodeCol::SelectionOutline:
			c.name = "SelectionOutline";
			c.col = ImColor(1.0f, 0.8f, 0.2f, 0.8f);
			break;
		case NodeCol::SelectionFill:
			c.name = "SelectionFill";
			c.col = ImColor(1.0f, 0.8f, 0.2f, 0.2f);
			break;
		}
		colours.push_back(c);
	}
}

NodeNetwork::NodeDependencyInformation::~NodeDependencyInformation()
{
	for (AbstractNode* n : nodes)
		delete n;
}
