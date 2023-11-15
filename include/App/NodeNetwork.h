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

	void DrawInput(const v2& cursor, const std::string& name, Node::NodeType type);
	void DrawOutput(const v2& cursor, float xOffset, const std::string& name, Node::NodeType type);
	float IOWidth(const std::string& text);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini);

	void DrawContextMenu();

private:
	const float NODE_ROUNDING = 4.0f;
	// allows for colour scheming
	const int NUM_COLOURS = 10;
	enum NodeCol {
		BGFill, BGOutline, BGHeader,
		IOBool, IOFloat,
		IO, IOSelected,
		Connector,
		Text,
		SelectedOutline
	};
	struct NodeColourData
	{
		std::string name;
		ImColor col;
	};
	std::vector<NodeColourData> colours;
	ImColor GetCol(NodeCol colour);
	ImColor GetCol(Node::NodeType type);
	void InitColours();

	ImDrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;
};