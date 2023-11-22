#pragma once
#include "Node.h"

#include <vector>
#include "imgui.h"

class NodeNetwork
{
public:
	inline NodeNetwork() { InitColours(); }
	~NodeNetwork();

	enum NodeCol {
		BGFill, BGOutline, BGHeader,
		IOBool, IOFloat,
		IO, IOSelected,
		Connector,
		Text,
		SelectedOutline, TopSelectedOutline, 
		SelectionOutline, SelectionFill
	};
	ImColor GetCol(NodeCol colour);
	ImColor GetCol(Node::NodeType type);

	void Draw(ImDrawList* drawList, class Canvas* canvas);
	inline void UnassignCanvas() { currentCanvas = nullptr; }

	Node* AddNodeFromName(const std::string& type, bool positionFromCursor = false);
	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr);
	std::vector<Node*> FindNodesInArea(const v2& p1, const v2& p2);

	void TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);

	void DrawInput(const v2& cursor, const std::string& name, Node::NodeType type);
	void DrawOutput(const v2& cursor, float xOffset, const std::string& name, Node::NodeType type);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini);
	void DrawConnection(const v2& target, const v2& origin, Node::NodeType type);

	void DrawContextMenu();

	inline void ClearDrawList() { currentList = nullptr; }

private:
	const float NODE_ROUNDING = 4.0f;
	// allows for colour scheming
	const int NUM_COLOURS = 13;
	struct NodeColourData
	{
		std::string name;
		ImColor col;
	};
	std::vector<NodeColourData> colours;
	void InitColours();

	ImDrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;
};