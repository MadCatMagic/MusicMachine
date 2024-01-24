#pragma once
#include "Node.h"
#include "Engine/DrawList.h"

#include <vector>
#include "imgui.h"
#include "BBox.h"

#define NODE_ROUNDING 4.0f

class NodeNetwork
{
public:
	static NodeNetwork* context;
	static void ExecuteCommand(std::vector<std::string> args);

	NodeNetwork();
	~NodeNetwork();

	void Draw(DrawList* drawList, class Canvas* canvas, std::vector<Node*>& selected, const bbox2& screen);
	inline void UnassignCanvas() { currentCanvas = nullptr; }

	Node* AddNodeFromName(const std::string& type, bool positionFromCursor = false);
	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr, size_t offset = 0);
	std::vector<Node*> FindNodesInArea(const v2& p1, const v2& p2);
	void PushNodeToTop(Node* node);

	void TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);
	void DeleteNode(Node* node);

	void DrawInput(const v2& cursor, const Node::NodeInput& inp, float width);
	void DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out);
	void DrawConnectionEndpoint(const v2& centre, DrawColour col, bool convertPosition = false, bool isNull = false);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset);
	void DrawConnection(const v2& target, const v2& origin, Node::NodeType type, Node* from, Node* to);

	void DrawContextMenu();

	bool Execute();

	inline void ClearDrawList() { currentList = nullptr; }
	inline void RecalculateDependencies() { recalculateDependencies = true; }

	DrawColour GetCol(Node::NodeType type);

private:

	// works out whether nodes have circular dependencies. will not calculate if circular dependencies exist.
	struct AbstractNode
	{
		unsigned int id = -1;
		// all index references to the master vector
		std::vector<size_t> markedBy;
		std::vector<size_t> outputs;
		std::vector<size_t> inputs;
		bool isEndpoint = false;
	};
	struct NodeDependencyInformation
	{
		~NodeDependencyInformation();
		std::vector<AbstractNode*> nodes;
		std::vector<Node*> endpoints;
		std::pair<size_t, size_t> problemConnection = std::make_pair<size_t, size_t>(0, 0);
		bool problemConnectionExists = false;
	};
	NodeDependencyInformation* CheckForCircularDependency();
	NodeDependencyInformation* nodeDependencyInfoPersistent = nullptr;
	bool recalculateDependencies = true;

	// for drawing later
	struct ConnectionToDraw
	{
		v2 a, b, c, d;
		DrawColour col;
		float thickness;
		size_t from;
		size_t to;
	};
	std::vector<ConnectionToDraw> connectionsToDraw;

	DrawList* currentList = nullptr;
	Canvas* currentCanvas = nullptr;

	std::vector<Node*> nodes;

	bool drawDebugInformation = false;
};