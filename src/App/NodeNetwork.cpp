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
		node->UI();
		node->CheckTouchedStatus();
		// automatically sets the size
		node->Draw(this);

		// draw node body
		v2 topLeft = canvas->ptcts(node->position);
		v2 bottomRight = canvas->ptcts(node->position + node->size);

		// rounded to 4 pixels - a single grid tile.
		drawList->ChannelsSetCurrent(0);
		drawList->AddRectFilled(
			topLeft.ImGui(),
			bottomRight.ImGui(),
			ImColor(GetCol(NodeCol::BGFill)),
			4.0f / canvas->GetSF().x,
			ImDrawFlags_RoundCornersAll
		);
		drawList->ChannelsSetCurrent(1);
	}
	drawList->ChannelsMerge();

	currentList = nullptr;
	currentCanvas = nullptr;
}

void NodeNetwork::AddNodeFromName(const std::string& type)
{
	Node* n = nullptr;

	if (type == "Node")
		n = new Node();

	if (n != nullptr)
		nodes.push_back(n);
}

Node* NodeNetwork::GetNodeAtPosition(const v2& pos, Node* currentSelection)
{
	if (currentSelection == nullptr)
	{
		for (Node* node : nodes)
			if (pos.inBox(node->position, node->position + node->size))
				return node;
	}
	return nullptr;
}

float NodeNetwork::DrawInput(const v2& cursor, const std::string& name, Node::NodeType type)
{
	ImColor colour = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
	if (type == Node::NodeType::Bool)
		colour = GetCol(NodeCol::IOBool);
	else if (type == Node::NodeType::Float)
		colour = GetCol(NodeCol::IOFloat);
	v2 pos = currentCanvas->ptcts(cursor);
	float sf = currentCanvas->GetSF().x;
	currentList->AddCircleFilled(pos.ImGui(), 4.0f / sf, GetCol(NodeCol::IO));
	currentList->AddCircleFilled(pos.ImGui(), 3.0f / sf, colour);
	pos = currentCanvas->ptcts(cursor + v2(8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), GetCol(NodeCol::Text), name.c_str());
	
	// return length
	return 12.0f * (float)name.size() + 12.0f;
}

void NodeNetwork::DrawContextMenu()
{
	if (ImGui::BeginMenu("Colours"))
	{
		for (int i = 0; i < NUM_COLOURS; i++)
			ImGui::ColorEdit4(colours[i].name.c_str(), &colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
		ImGui::EndMenu();
	}
}

ImColor NodeNetwork::GetCol(NodeCol colour)
{
	return colours[(int)colour].col;
}

void NodeNetwork::InitColours()
{
	for (int i = 0; i < NUM_COLOURS; i++)
	{
		/*BGFill, BGOutline,
		IOBool, IOFloat,
		IO, IOSelected,
		Connector,
		Text*/
		NodeColourData c;
		switch ((NodeCol)i)
		{
		case NodeCol::BGFill:
			c.name = "BGFill";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
			break;

		case NodeCol::BGOutline:
			c.name = "BGOutline";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
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
		}
		colours.push_back(c);
	}
}
