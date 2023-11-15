#pragma once
#include "Vector.h"
#include <vector>

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

struct Node
{
	enum NodeType {
		Bool, Float
	};

	friend class NodeNetwork;

	v2 position = v2::zero;
	// please don't set me! thanks xx
	v2 size = v2::zero;

	bool HandleClick(const v2& nodePos);

protected:
	virtual void Init();
	virtual void IO();
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
	
	void Draw(class NodeNetwork* network);
	void UpdateDimensions();
	float IOWidth(const std::string& text);
};