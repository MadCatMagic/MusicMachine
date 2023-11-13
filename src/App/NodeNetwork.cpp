#include "App/NodeNetwork.h"
#include "App/Canvas.h"
#include "imgui.h"

NodeNetwork::~NodeNetwork()
{
	for (Node* n : nodes)
		delete n;
	nodes.clear();
}

void NodeNetwork::AddNodeFromName(const std::string& type)
{
	Node* n = nullptr;

	if (type == "Node")
	{
		n = new Node();
	}

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

void NodeNetwork::DrawInput(const v2& cursor, const std::string& name, Node::NodeType type)
{
	v3 colour = 0.2f;
	if (type == Node::NodeType::Bool)
		colour = v3(1.0f, 0.2f, 0.6f);
	else if (type == Node::NodeType::Float)
		colour = v3(0.4f, 0.6f, 0.9f);
	v2 pos = currentCanvas->ptcts(cursor);
	currentList->AddCircleFilled(pos.ImGui(), 5.0f / currentCanvas->GetSF().x, ImColor(colour.x, colour.y, colour.z));
	pos = currentCanvas->ptcts(cursor + v2(8.0f, -6.0f));
	currentList->AddText(pos.ImGui(), ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Text)), name.c_str());
}

void NodeNetwork::Draw(ImDrawList* drawList, Canvas* canvas)
{
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
		drawList->AddRectFilled(
			bottomRight.ImGui(),
			topLeft.ImGui(),
			ImColor(ImGui::GetStyleColorVec4(ImGuiCol_WindowBg)),
			4.0f / canvas->GetSF().x,
			ImDrawFlags_RoundCornersAll
		);
	}

	currentList = nullptr;
	currentCanvas = nullptr;
}
