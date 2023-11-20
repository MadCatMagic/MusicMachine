#include "App/NodeNetwork.h"
#include "App/Canvas.h"
#include "imgui.h"

NodeNetwork::~NodeNetwork()
{
	for (Node* n : nodes)
		delete n;
	nodes.clear();
}

void NodeNetwork::Draw(ImDrawList* drawList, Canvas* canvas)
{
	drawList->ChannelsSplit(2);
	drawList->ChannelsSetCurrent(1);
	currentList = drawList;
	currentCanvas = canvas;
	for (Node* node : nodes)
	{
		// make sure to add the correct node connections to the buffer
		node->ResetTouchedStatus();
		node->IO();
		node->CheckTouchedStatus();
		// automatically sets the size
		node->Draw(this);

		// draw node body
		v2 topLeft = canvas->ptcts(node->position);
		v2 bottomRight = canvas->ptcts(node->position + node->size);
		v2 tlO = canvas->ptcts(node->position - 1.0f);
		v2 brO = canvas->ptcts(node->position + node->size + 1.0f);

		ImColor outline = GetCol(node->selected ? NodeCol::SelectedOutline : NodeCol::BGOutline);
		// rounded to 4 pixels - a single grid tile.
		drawList->ChannelsSetCurrent(0);
		drawList->AddRectFilled(
			tlO.ImGui(),
			brO.ImGui(),
			ImColor(outline),
			NODE_ROUNDING / canvas->GetSF().x,
			ImDrawFlags_RoundCornersAll
		);
		drawList->AddRectFilled(
			topLeft.ImGui(),
			bottomRight.ImGui(),
			ImColor(GetCol(NodeCol::BGFill)),
			NODE_ROUNDING / canvas->GetSF().x,
			ImDrawFlags_RoundCornersAll
		);
		drawList->ChannelsSetCurrent(1);
	}
	drawList->ChannelsMerge();

	currentList = nullptr;
}

#include "App/NodeTypes.h"
Node* NodeNetwork::AddNodeFromName(const std::string& type, bool positionFromCursor)
{
	Node* n = nullptr;

	if (type == "Node")
		n = new Node();

	if (type == "Maths")
		n = new MathsNode();

	if (n == nullptr)
		return nullptr;

	n->Init();
	n->UpdateDimensions();
	if (positionFromCursor)
		n->position = currentCanvas->CanvasToPosition(currentCanvas->ScreenToCanvas(ImGui::GetMousePos())) - n->size * 0.5f;
	nodes.push_back(n);
	return n;
}

Node* NodeNetwork::GetNodeAtPosition(const v2& pos, Node* currentSelection)
{
	if (currentSelection == nullptr)
	{
		for (Node* node : nodes)
			if (pos.inBox(node->position - v2(4.0f), node->position + node->size + v2(4.0f)))
				return node;
	}
	return nullptr;
}

void NodeNetwork::TryEndConnection(Node* origin, const std::string& originName, const v2& pos)
{
	for (Node* n : nodes)
		if (n != origin && n->TryConnect(origin, originName, pos))
			return;
}

void NodeNetwork::DrawInput(const v2& cursor, const std::string& name, Node::NodeType type)
{
	ImColor colour = GetCol(type);
	v2 pos = currentCanvas->ptcts(cursor);
	float sf = currentCanvas->GetSF().x;
	// input circle thingy
	currentList->AddCircleFilled(pos.ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
	currentList->AddCircleFilled(pos.ImGui(), 3.0f / sf, colour);
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
	currentList->AddCircleFilled(pos.ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
	currentList->AddCircleFilled(pos.ImGui(), 3.0f / sf, colour);
	// text
	pos = currentCanvas->ptcts(cursor + v2(xOffset - (name.size() + 1) * 6.0f, 0.0f) + v2(-8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), name.c_str());
}

void NodeNetwork::DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini)
{
	v2 topLeft = currentCanvas->ptcts(cursor + 1.0f);
	v2 bottomRight = currentCanvas->ptcts(cursor + v2(width, height) - 1.0f);
	v2 textPos = currentCanvas->ptcts(v2(cursor.x + 8.0f, cursor.y + height * 0.5f - 6.0f));
	ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
	currentList->AddRectFilled(topLeft.ImGui(), bottomRight.ImGui(), GetCol(NodeCol::BGHeader), NODE_ROUNDING / currentCanvas->GetSF().x, flags);
	currentList->AddText(textPos.ImGui(), GetCol(NodeCol::Text), name.c_str());

	// minimized triangle thing
	v2 triCentre = cursor + v2(width - 8.0f, height * 0.5f);
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
	float width = 10.0f + fabsf(target.y - origin.y) * 0.4f;
	currentList->AddBezierCubic(
		currentCanvas->ptcts(origin).ImGui(),
		currentCanvas->ptcts(origin + v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target - v2(width, 0.0f)).ImGui(),
		currentCanvas->ptcts(target).ImGui(),
		GetCol(type),
		1.5f / currentCanvas->GetSF().x
	);
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
			"Maths"
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
		}
		colours.push_back(c);
	}
}
