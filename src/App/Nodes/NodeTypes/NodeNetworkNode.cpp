#include "App/Nodes/NodeTypes/NodeNetworkNode.h"

void NodeNetworkNode::Init()
{
	name = "NodeNetworkNode";
	title = "Network";
	minSpace = v2(40.0f, 20.0f);
}

void NodeNetworkNode::IO()
{
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
