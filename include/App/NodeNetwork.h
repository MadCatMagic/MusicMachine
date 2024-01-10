#pragma once
#include "Node.h"

#include <vector>
#include "imgui.h"
#include "BBox.h"

#define NODE_ROUNDING 4.0f

class NodeNetwork
{
public:
	NodeNetwork();
	~NodeNetwork();

	enum NodeCol {
		BGFill, BGOutline, BGHeader,
		IOBool, IOFloat,
		IO, IOSelected,
		Connector,
		Text,
		SelectedOutline, TopSelectedOutline, SelectedFill,
		SelectionOutline, SelectionFill
	};
	ImColor GetCol(NodeCol colour);
	ImColor GetCol(Node::NodeType type);

	void Draw(ImDrawList* drawList, class Canvas* canvas, std::vector<Node*>& selected, const bbox2& screen);
	inline void UnassignCanvas() { currentCanvas = nullptr; }

	Node* AddNodeFromName(const std::string& type, bool positionFromCursor = false);
	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr, size_t offset = 0);
	std::vector<Node*> FindNodesInArea(const v2& p1, const v2& p2);
	void PushNodeToTop(Node* node);

	void TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);
	void DeleteNode(Node* node);

	void DrawInput(const v2& cursor, const std::string& name, Node::NodeType type);
	void DrawOutput(const v2& cursor, float xOffset, const std::string& name, Node::NodeType type);
	void DrawConnectionEndpoint(const v2& centre, const ImColor& color, bool convertPosition = false);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset);
	void DrawConnection(const v2& target, const v2& origin, Node::NodeType type);

	void DrawContextMenu();

	inline void ClearDrawList() { currentList = nullptr; }

private:
	// works out whether nodes have circular dependencies. will not calculate if circular dependencies exist.
	struct AbstractNode
	{
		unsigned int id = -1;
		std::vector<AbstractNode*> markedBy;
		std::vector<AbstractNode*> outputs;
		std::vector<AbstractNode*> inputs;
	};
	void CheckForCircularDependency();

	// allows for colour scheming
	const int NUM_COLOURS = 14;
	struct NodeColourData
	{
		std::string name;
		ImColor col;
	};
	std::vector<NodeColourData> colours;
	void InitColours();

	// for drawing later
	struct ConnectionToDraw
	{
		ImVec2 a, b, c, d;
		ImColor col;
		float thickness;
	};
	std::vector<ConnectionToDraw> connectionsToDraw;

	ImDrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;
};