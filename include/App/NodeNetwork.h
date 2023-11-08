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

private:
	ImDrawList* currentList = nullptr;

	std::vector<Node*> nodes;
};