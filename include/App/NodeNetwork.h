#pragma once
#include "Node.h"

#include <vector>

class NodeNetwork
{
public:
	~NodeNetwork();

	void Draw(struct ImDrawList* drawList, class Canvas* canvas);

	void AddNodeFromName(const std::string& type);

private:
	ImDrawList* currentList = nullptr;

	std::vector<Node*> nodes;
};