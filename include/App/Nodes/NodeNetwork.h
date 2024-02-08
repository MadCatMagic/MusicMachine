#pragma once
#include "App/Nodes/Node.h"
#include "Engine/DrawList.h"

#include <vector>
#include "imgui.h"
#include "BBox.h"

#define NODE_ROUNDING 4.0f

class NodeNetwork
{
public:
	friend class NodeNetworkRenderer;

	static NodeNetwork* context;
	static void ExecuteCommand(std::vector<std::string> args);

	NodeNetwork();
	NodeNetwork(const std::string& nnFilePath);
	~NodeNetwork();

	Node* AddNodeFromName(const std::string& type, bool positionFromCursor = false);
	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr, size_t offset = 0);
	std::vector<Node*> FindNodesInArea(const v2& p1, const v2& p2);
	void PushNodeToTop(Node* node);

	void TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);
	void DeleteNode(Node* node);

	void DrawContextMenu();

	bool Execute();
	void Update();

	void SaveNetworkToFile(const std::string& nnFilePath);

	inline void RecalculateDependencies() { recalculateDependencies = true; }
	inline void UnassignCanvas() { currentCanvas = nullptr; }
	inline void AssignCanvas(class Canvas* canvas) { currentCanvas = canvas; }

	Node* GetNodeFromID(const std::string& id);

private:

	Node* CreateRawNode(const std::string& type);

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

	std::vector<Node*> nodes;
	std::unordered_map<std::string, Node*> nodeIDMap;

	bool drawDebugInformation = false;

	class Canvas* currentCanvas = nullptr;
};

class NodeNetworkRenderer
{
public:
	inline NodeNetworkRenderer(NodeNetwork* network, class Canvas* canvas) : canvas(canvas), network(network) { }
	
	void Draw(DrawList* drawList, std::vector<Node*>& selected, const bbox2& screen);
	
	void DrawConnection(const v2& target, const v2& origin, Node::NodeType type, Node* from, Node* to);
	
	inline void UnassignCanvas() { canvas = nullptr; }

private:
	NodeNetwork* network;

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
	Canvas* canvas;
	bool drawDebugInformation = false;

	void DrawNode(Node* node, bool cullBody);

	void DrawInput(const v2& cursor, const Node::NodeInput& inp, float width);
	void DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out);
	void DrawConnectionEndpoint(const v2& centre, DrawColour col, bool convertPosition = false, bool isNull = false);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset);
	
	DrawColour GetCol(Node::NodeType type);
};