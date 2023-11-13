#pragma once
#include "Node.h"

#include <vector>

class NodeNetwork
{
public:
	~NodeNetwork();

	void Draw(struct ImDrawList* drawList, class Canvas* canvas);

	void AddNodeFromName(const std::string& type);

	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr);

	void DrawInput(const v2& cursor, const std::string& name, Node::NodeType type);

private:
	ImDrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;
};