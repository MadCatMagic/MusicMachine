#pragma once
#include "App/Nodes/Node.h"
#include "Engine/DrawList.h"

#include "App/AudioStream.h"

#include "App/Arranger.h"

#include "imgui.h"
#include "BBox.h"

#include <bitset>

#define NODE_ROUNDING 4.0f

class NodeNetwork
{
public:
	friend class NodeNetworkRenderer;
	friend Node;

	NodeNetwork();
	NodeNetwork(const std::string& nnFilePath);
	~NodeNetwork();

	Node* AddNodeFromName(const std::string& type, const v2& initPos = v2::zero, bool skipInitialisation = false);
	Node* GetNodeAtPosition(const v2& pos, Node* currentSelection = nullptr, size_t offset = 0);
	std::vector<Node*> FindNodesInArea(const v2& p1, const v2& p2);
	void PushNodeToTop(Node* node);

	void TryEndConnection(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);
	void DeleteNode(Node* node);

	std::pair<bool, bool> DrawContextMenu(const v2& contextMenuClickPos);

	bool Execute(bool setAudioStream, int ownedID);
	void Update();

	void SaveNetworkToFile(const std::string& nnFilePath);

	inline void RecalculateDependencies() { recalculateDependencies = true; }
	inline bool doIDrawDebug() const { return drawDebugInformation; }

	Node* GetNodeFromID(const std::string& id);

	AudioStream* audioStream = nullptr;

	std::string name = "new network";
	std::bitset<16> usedInNetworkNode;

	bool isRoot = false;
	std::vector<struct NodeNetworkVariable*> ioVariables;

private:

	// works out whether nodes have circular dependencies. will not calculate if circular dependencies exist.
	struct AbstractNode
	{
		unsigned int id = -1;
		bool onStack = false;
		// all index references to the master vector
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
	std::pair<bool, std::pair<size_t, size_t>> TarjanSCCSearch(size_t node, std::vector<size_t>& stack, std::vector<AbstractNode*>& nodes);
	NodeDependencyInformation* nodeDependencyInfoPersistent = nullptr;
	bool recalculateDependencies = true;

	std::vector<Node*> nodes;
	std::unordered_map<std::string, Node*> nodeIDMap;

	bool drawDebugInformation = false;
};

class NodeNetworkRenderer
{
public:
	inline NodeNetworkRenderer(NodeNetwork* network, class Canvas* canvas) : canvas(canvas), network(network) { }
	
	void Draw(DrawList* drawList, std::vector<Node*>& selected, const bbox2& screen);
	void DrawConnection(const v2& target, const v2& origin, Node::NodeType type, Node* from, Node* to);
	
	inline void UnassignCanvas() { canvas = nullptr; }

	bool drawDebugInformation = false;

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

	void DrawNode(Node* node, bool cullBody);

	void DrawInput(const v2& cursor, const Node::NodeInput& inp, float width);
	void DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out);
	void DrawConnectionEndpoint(const v2& centre, DrawColour col, bool convertPosition = false, bool isNull = false);
	void DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset);
	
	DrawColour GetCol(Node::NodeType type);
};