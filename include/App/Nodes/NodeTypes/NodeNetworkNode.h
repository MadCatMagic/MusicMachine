#pragma once
#include "App/Nodes/Node.h"

struct NodeNetworkNode : public Node
{
public:
	void AssignNetwork(NodeNetwork* network);

protected:
	~NodeNetworkNode();

	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

	class NodeNetwork* network = nullptr;
};