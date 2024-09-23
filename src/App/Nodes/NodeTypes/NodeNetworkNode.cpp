#include "App/Nodes/NodeTypes/NodeNetworkNode.h"
#include "App/Nodes/NodeTypes/NodeNetworkVariable.h"
#include "App/Nodes/NodeNetwork.h"

void NodeNetworkNode::AssignNetwork(NodeNetwork* network)
{
	if (this->network != nullptr)
		this->network->usedInNetworkNode -= 1;
	this->network = network;
	this->network->usedInNetworkNode += 1;

}

NodeNetworkNode::~NodeNetworkNode()
{
	if (network != nullptr)
		network->usedInNetworkNode -= 1;
}

void NodeNetworkNode::Init()
{
	name = "NodeNetworkNode";
	title = "Network";
	minSpace = v2(40.0f, 20.0f);
	if (network != nullptr)
		network->usedInNetworkNode += 1;
}

void NodeNetworkNode::IO()
{
	if (network == nullptr)
		return;
	for (size_t i = 0; i < network->ioVariables.size(); i++)
	{
		NodeNetworkVariable* v = network->ioVariables[i];
		if (v->isOutput)
		{
			DefaultOutput((v->id + std::to_string(i)).c_str(), nullptr, v->nodeType);
		}
		else
			DefaultInput((v->id + std::to_string(i)).c_str(), nullptr, v->nodeType);
	}
}

void NodeNetworkNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
}

bool NodeNetworkNode::OnClick(const NodeClickInfo& info)
{
    return false;
}

void NodeNetworkNode::Work()
{
}

void NodeNetworkNode::Load(JSONType& data)
{
}

JSONType NodeNetworkNode::Save()
{
    return JSONType();
}
