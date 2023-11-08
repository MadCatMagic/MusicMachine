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

void NodeNetwork::Draw(ImDrawList* drawList, Canvas* canvas)
{
	currentList = drawList;
	for (Node* node : nodes)
	{
		// draw node body
		v2 topLeft = canvas->ptcts(node->position);
		v2 bottomRight = canvas->ptcts(node->position + node->GetBounds());
		
		drawList->AddRectFilled(
			bottomRight.ImGui(),
			topLeft.ImGui(),
			ImColor(120, 120, 120, 255)
		);

		node->Draw(this);
	}

	currentList = nullptr;
}
