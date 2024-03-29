#pragma once
#include "Vector.h"
#include "BBox.h"
#include <vector>
#include "App/JSON.h"

struct Node;

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

enum NodeClickResponseType {
	None, Minimise, BeginConnection, BeginConnectionReversed, 
	InteractWithFloatSlider, InteractWithIntSlider, InteractWithBool
};
struct NodeClickResponse
{
	bool handled = false;
	NodeClickResponseType type = NodeClickResponseType::None;
	std::string originName = "";
	Node* origin = nullptr;
	union sliderValueType {
		float* f;
		int* i;
	};

	sliderValueType sliderValue = { };
	float sliderDelta = 0.0f;
};

/*
-- Inheriting from the Node class --
override 'void IO()' to create your own inputs and outputs.
- inputs and outputs can share names, but no two inputs should have the same name AND type, and the same for two outputs.
- they will be displayed in the order you call them in this function

override 'void Init()' instead of using a constructor for anything which needs to be done before runtime but after initialisation
override 'void Work()' to actually serve your function, taking your inputs and turning them into the output. Called every frame.

do not use a regular constructor pwease
in Init assign 'name' and 'minSpace' do declare, respectively, the name of the node and the minimum space it wants for ui.
*/
struct Node
{
	enum NodeType {
		Bool, Float, Int
	};

	friend class NodeNetwork;
	friend class NodeNetworkRenderer;
	virtual void IO();

	v2 position = v2::zero;
	// please don't set me! thanks xx
	v2 size = v2::zero;

	NodeClickResponse HandleClick(const v2& nodePos);

	bool Connect(size_t inputIndex, Node* origin, size_t originIndex);
	void Disconnect(size_t inputIndex);

	v2 GetInputPos(const std::string& name) const;
	v2 GetOutputPos(const std::string& name) const;
	NodeType GetInputType(const std::string& name) const;
	NodeType GetOutputType(const std::string& name) const;

protected:
	inline virtual void Init() { }
	inline virtual void Work() { }
	inline virtual std::string Result() { return "Null"; }

	inline virtual JSONType Save() { return JSONType(JSONType::Object); }
	inline virtual void Load(JSONType& data) { }

	std::string name = "Node";
	std::string title = "Base Node";
	v2 minSpace = v2(20, 20);

	// to be called in InitializeUI()
	void BoolInput(const std::string& name, bool* target);
	void BoolOutput(const std::string& name, bool* target);

	void FloatInput(const std::string& name, float* target, float min = 0.0f, float max = 1.0f);
	void FloatOutput(const std::string& name, float* target);

	void IntInput(const std::string& name, int* target, int min = 0, int max = 127);
	void IntOutput(const std::string& name, int* target);

private:
	uint64_t id = 0;
	std::string id_s();

	bool mini = false;
	const float headerHeight = 20.0f;
	const float miniTriangleOffset = 10.0f;

	NodeNetwork* parent = nullptr;
	bool hasBeenExecuted = true;
	void Execute();

	struct NodeOutput
	{
		std::string name;
		void* data{};
		NodeType type{};

		size_t connections = 0;
		bool touchedThisFrame = false;
	};

	struct NodeInput
	{
		std::string name;
		void* target{};
		NodeType type{};

		float fmin = 0.0f;
		float fmax = 1.0f;

		Node* source = nullptr;
		std::string sourceName = "";
		bool touchedThisFrame = false;
	};

	std::vector<NodeInput> inputs;
	std::vector<NodeOutput> outputs;

	void TransferInput(const NodeInput& i);
	void TransferOutput(const NodeOutput& i);
	void ResetTouchedStatus();
	void CheckTouchedStatus();

	inline void NodeInit(NodeNetwork* parent, uint64_t id) { this->parent = parent; this->id = id; }
	
	void LoadData(JSONType& data);
	JSONType SaveData();

	bool TryConnect(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);
	
	v2 GetInputPos(size_t index) const;
	v2 GetOutputPos(size_t index) const;
	size_t GetInputIndex(const std::string& name) const;
	size_t GetOutputIndex(const std::string& name) const;
	void UpdateDimensions();
	float IOWidth(const std::string& text, size_t additionalWidth = 0) const;
	float headerSize() const;
	float getNormalWidth() const;
	bbox2 getBounds() const;

	size_t DataSize(NodeType type);
};