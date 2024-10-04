#pragma once
#include "Vector.h"
#include "BBox.h"
#include "App/JSON.h"

#include "App/AudioChannel.h"
#include "App/Sequencer.h"

struct Node;

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

enum NodeClickResponseType {
	None, Interact, BeginConnection, BeginConnectionReversed, 
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
	bool sliderLockMin = false;
	bool sliderLockMax = false;
	float sliderMin = 0.0f;
	float sliderMax = 1.0f;

	sliderValueType sliderValue = { };
	float sliderDelta = 0.0f;
};

class NodeNetwork;

struct NodeClickInfo
{
	v2 pos;
	// 0 is on click
	// 1 is on hold
	// 2 is on release
	int interactionType = 0;
	bool isRight = false;
};

struct NodeRenderer
{
	const float headerHeight = 20.0f;
	const float miniTriangleOffset = 10.0f;

	inline NodeRenderer(Node* n) : node(n) {}

	v2 GetInputPos(size_t index) const;
	v2 GetOutputPos(size_t index) const;

	v2 spaceOffset() const;
	void UpdateDimensions();
	float IOWidth(const std::string& text, size_t additionalWidth = 0) const;
	float headerSize() const;
	float getNormalWidth() const;
	bbox2 getBounds() const;

	v2 size;

private:
	Node* node;
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
	inline virtual ~Node() { }

	enum NodeType {
		Bool, Float, Int, Audio, Sequencer
	};

	enum FloatDisplayType { None, Db, Hz, TempoSync };

	friend NodeNetwork;
	friend class NodeNetworkRenderer;
	friend struct NodeRenderer;

	virtual void IO();

	v2 position = v2::zero;

	NodeClickResponse HandleClick(const NodeClickInfo& info);

	bool Connect(size_t inputIndex, Node* origin, size_t originIndex);
	void Disconnect(size_t inputIndex);
	void DisconnectOutput(size_t outputIndex);

	NodeType GetInputType(const std::string& name) const;
	NodeType GetOutputType(const std::string& name) const;
	v2 GetInputPos(const std::string& name) const;
	v2 GetOutputPos(const std::string& name) const;

	NodeRenderer renderer = NodeRenderer(this);

protected:
	inline virtual void Init() { }
	inline virtual void Work() { }
	inline virtual AudioChannel* Result() { return nullptr; }

	inline virtual void Render(const v2& topLeft, class DrawList* dl, bool lodOn) { }
	// returns whether the click was used or not
	inline virtual bool OnClick(const NodeClickInfo& info) { return false; }

	inline virtual JSONType Save() { return JSONType(JSONType::Object); }
	inline virtual void Load(JSONType& data) { }

	std::string name = "Node";
	std::string title = "Base Node";
	v2 minSpace = v2(20, 20);

	// to be called in InitializeUI()
	void BoolInput(const std::string& name, bool* target);
	void BoolOutput(const std::string& name, bool* target);

	void FloatInput(const std::string& name, float* target, float min = 0.0f, float max = 1.0f, bool lockMinToRange = false, bool lockMaxToRange = false, FloatDisplayType displayType = FloatDisplayType::None);
	void FloatOutput(const std::string& name, float* target);

	void TempoSyncIntInput(const std::string& name, int* target);
	void IntInput(const std::string& name, int* target, int min = 0, int max = 127, bool lockMinToRange = false, bool lockMaxToRange = false);
	void IntOutput(const std::string& name, int* target);

	void AudioInput(const std::string& name, AudioChannel* target);
	void AudioOutput(const std::string& name, AudioChannel* target);

	void SequencerInput(const std::string& name, PitchSequencer* target);
	void SequencerOutput(const std::string& name, PitchSequencer* target);

	void DefaultInput(const std::string& name, bool* b, int* i, float* f, AudioChannel* c, PitchSequencer* s, NodeType type);
	void DefaultOutput(const std::string& name, bool* b, int* i, float* f, AudioChannel* c, PitchSequencer* s, NodeType type);

	// returns in terms of beats
	float tempoSyncToFloat(int v) const;

	NodeNetwork* parent = nullptr;

private:
	uint64_t id = 0;
	std::string id_s();

	bool mini = false;

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
		bool lockMin = false;
		bool lockMax = false;

		Node* source = nullptr;
		std::string sourceName = "";
		bool touchedThisFrame = false;

		FloatDisplayType displayType = FloatDisplayType::None;
	};

	std::vector<NodeInput> inputs;
	std::vector<NodeOutput> outputs;

	void TransferInput(const NodeInput& i);
	void TransferOutput(const NodeOutput& i);
	void ResetTouchedStatus();
	void CheckTouchedStatus();

	size_t GetInputIndex(const std::string& name) const;
	size_t GetOutputIndex(const std::string& name) const;

	inline void NodeInit(NodeNetwork* parent, uint64_t id) { this->parent = parent; this->id = id; }
	
	void LoadData(JSONType& data);
	JSONType SaveData();

	bool TryConnect(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed);

	size_t DataSize(NodeType type);
};