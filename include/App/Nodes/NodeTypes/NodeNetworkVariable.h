#pragma once
#include "App/Nodes/Node.h"

struct NodeNetworkVariable : public Node
{
	friend struct NodeNetworkNode;
	friend class NodeNetwork;
	~NodeNetworkVariable();

	bool isOutput = false;
	std::string id = "new variable";

protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info);

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	NodeType nodeType = NodeType::Audio;

	bool b = false;
	int i = 0;
	float f = 0.0f;
	AudioChannel c;
	PitchSequencer s;
};