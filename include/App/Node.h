#pragma once
#include "Vector.h"
#include <vector>

struct Node;

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

enum NodeClickResponseType {
	None, Minimise, BeginConnection
};
struct NodeClickResponse
{
	bool handled = false;
	NodeClickResponseType type = NodeClickResponseType::None;
	std::string originName = "";
	Node* origin = nullptr;
};

struct Node
{
	enum NodeType {
		Bool, Float
	};

	friend class NodeNetwork;
	virtual void IO();

	v2 position = v2::zero;
	// please don't set me! thanks xx
	v2 size = v2::zero;

	NodeClickResponse HandleClick(const v2& nodePos);

	bool Connect(size_t inputIndex, Node* origin, size_t originIndex);
	void Disconnect(size_t inputIndex);

	v2 GetInputPos(const std::string& name) const;
	v2 GetOutputPos(const std::string& name) const;
	NodeType GetOutputType(const std::string& name) const;

protected:
	virtual void Init();
	//virtual void IO();
	inline virtual void Work() { }

	std::string name = "Node";
	v2 minSpace = v2(20, 20);

	// to be called in InitializeUI()
	bool BoolInput(const std::string& name, bool* target);
	bool BoolOutput(const std::string& name, bool* target);

	bool FloatInput(const std::string& name, float* target);
	bool FloatOutput(const std::string& name, float* target);

private:
	bool selected = false;
	bool mini = false;
	const float headerHeight = 20.0f;

	struct NodeOutput
	{
		std::string name;
		void* data;
		NodeType type;
		int connections = 0;
	};

	struct NodeInput
	{
		std::string name;
		void* target;
		NodeType type;

		Node* source = nullptr;
		std::string sourceName = "";
		bool touchedThisFrame = false;
	};

	std::vector<NodeInput> inputs;
	std::vector<NodeOutput> outputs;

	void TransferInput(const NodeInput& i);
	void ResetTouchedStatus();
	void CheckTouchedStatus();

	bool TryConnect(Node* origin, const std::string& originName, const v2& pos);
	
	v2 GetInputPos(size_t index) const;
	v2 GetOutputPos(size_t index) const;
	size_t GetOutputIndex(const std::string& name) const;
	void Draw(class NodeNetwork* network);
	void UpdateDimensions();
	float IOWidth(const std::string& text);
};