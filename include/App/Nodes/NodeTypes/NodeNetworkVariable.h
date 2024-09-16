#pragma once
#include "App/Nodes/Node.h"

struct NodeNetworkVariable : public Node
{
	friend class NodeNetworkNode;
	friend class NodeNetwork;
	~NodeNetworkVariable();

protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info);

private:
	bool isOutput = false;

	AudioChannel channel;
	std::string id = "new variable";
};