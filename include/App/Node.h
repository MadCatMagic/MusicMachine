#pragma once
#include "Vector.h"
#include <vector>

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

struct Node
{
	friend class NodeNetwork;

	v2 position = v2::zero;
	// please don't set me! thanks xx
	v2 size = v2::zero;

protected:
	virtual void Init();
	virtual void UI();

	std::string name = "Node";
	v2 minSpace = v2(20, 20);

	// to be called in InitializeUI()
	bool IntInput(const std::string& name, int* target);
	bool IntOutput(const std::string& name, int* target);

	bool FloatInput(const std::string& name, float* target);
	bool FloatOutput(const std::string& name, float* target);

private:
	enum NodeType {
		Int32, Float
	};

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
};