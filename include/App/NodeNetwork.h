#pragma once
#include "Node.h"

#include <vector>
#include "imgui.h"

class NodeNetwork
{
public:
	inline NodeNetwork() { InitColours(); }
	~NodeNetwork();

	void Draw(ImDrawList* drawList, class Canvas* canvas);

	void AddNodeFromName(const std::string& type);

	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr);

	float DrawInput(const v2& cursor, const std::string& name, Node::NodeType type);

	void DrawContextMenu();

private:
	// allows for colour scheming
	const int NUM_COLOURS = 8;
	enum NodeCol {
		BGFill, BGOutline,
		IOBool, IOFloat,
		IO, IOSelected,
		Connector,
		Text
	};
	struct NodeColourData
	{
		std::string name;
		ImColor col;
	};
	std::vector<NodeColourData> colours;
	ImColor GetCol(NodeCol colour);
	void InitColours();

	ImDrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;
};