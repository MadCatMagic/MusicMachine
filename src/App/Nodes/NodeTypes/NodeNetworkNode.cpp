#include "App/Nodes/NodeTypes/NodeNetworkNode.h"
#include "App/Nodes/NodeTypes/NodeNetworkVariable.h"
#include "App/Nodes/NodeNetwork.h"

#include "App/App.h"

void NodeNetworkNode::AssignNetwork(NodeNetwork* network)
{
	if (this->network != nullptr)
		this->network->usedInNetworkNode -= 1;
	this->network = network;
	this->network->usedInNetworkNode += 1;
	title = network->name;
	network->RecalculateDependencies();
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
	{
		network->usedInNetworkNode += 1;
		title = network->name;
		network->RecalculateDependencies();
	}
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
			DefaultOutput(v->id.c_str(), &v->b, &v->i, &v->f, &v->c, &v->s, v->nodeType);
		}
		else
			DefaultInput(v->id.c_str(), &v->b, &v->i, &v->f, &v->c, &v->s, v->nodeType);
	}
}

void NodeNetworkNode::Work()
{
	network->Execute(false);
}

void NodeNetworkNode::Load(JSONType& data)
{
	AssignNetwork(App::instance->GetNetwork(data.s));
}

JSONType NodeNetworkNode::Save()
{
    return JSONType(title);
}
