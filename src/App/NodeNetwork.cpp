#include "App/NodeNetwork.h"
#include "App/Canvas.h"
#include "imgui.h"

#include "BBox.h"

NodeNetwork::NodeNetwork()
{
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
		bool dontCullNode = screen.overlaps(bbox2(node->position, node->position + node->size));
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

	// draw connections on top of all the nodes
	for (const ConnectionToDraw& connection : connectionsToDraw)
		drawList->AddBezierCubic(connection.a, connection.b, connection.c, connection.d, connection.col, connection.thickness);
	connectionsToDraw.clear();
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

	if (n == nullptr)
		return nullptr;

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
	if (index != nodes.end())
	{
		nodes.erase(index);
		nodes.push_back(node);
	}
}

void NodeNetwork::TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed)
{
	for (Node* n : nodes)
		if (n != origin && n->TryConnect(origin, originName, pos, connectionReversed))
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
}

void NodeNetwork::DrawInput(const v2& cursor, const std::string& name, Node::NodeType type)
{
	ImColor colour = GetCol(type);
	v2 pos = currentCanvas->ptcts(cursor);
	float sf = currentCanvas->GetSF().x;
	// input circle thingy
	DrawConnectionEndpoint(pos, colour);
	// text
	pos = currentCanvas->ptcts(cursor + v2(8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), name.c_str());
}

void NodeNetwork::DrawOutput(const v2& cursor, float xOffset, const std::string& name, Node::NodeType type)
{
	ImColor colour = GetCol(type);
	v2 pos = currentCanvas->ptcts(cursor + v2(xOffset, 0.0f));
	float sf = currentCanvas->GetSF().x;
	// draw on right side of node
	DrawConnectionEndpoint(pos, colour);
	// text
	pos = currentCanvas->ptcts(cursor + v2(xOffset - (name.size() + 1) * 6.0f, 0.0f) + v2(-8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), name.c_str());
}

void NodeNetwork::DrawConnectionEndpoint(const v2& centre, const ImColor& color, bool convertPosition)
{
	// draw it
	float sf = currentCanvas->GetSF().x;
	if (convertPosition)
	{
		currentList->AddCircleFilled(currentCanvas->ptcts(centre).ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
		currentList->AddCircleFilled(currentCanvas->ptcts(centre).ImGui(), 3.0f / sf, color);
	}
	else
	{
		currentList->AddCircleFilled(centre.ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
		currentList->AddCircleFilled(centre.ImGui(), 3.0f / sf, color);
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

void NodeNetwork::DrawConnection(const v2& target, const v2& origin, Node::NodeType type)
{
	float width = 12.0f + fabsf(target.x - origin.x) * 0.3f + fabsf(target.y - origin.y) * 0.1f;
	ConnectionToDraw c{
		currentCanvas->ptcts(origin).ImGui(),
		currentCanvas->ptcts(origin + v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target - v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target).ImGui(),
		GetCol(type),
		1.5f / currentCanvas->GetSF().x
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
			"Long"
		};
		for (const std::string& name : nodeNames)
			if (ImGui::MenuItem(name.c_str()))
				AddNodeFromName(name, true);
		ImGui::EndMenu();
	}
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
