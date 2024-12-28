#include "App/Nodes/NodeTypes/NodeNetworkVariable.h"
#include "App/Nodes/NodeNetwork.h"

NodeNetworkVariable::~NodeNetworkVariable()
{
	auto p = std::find(parent->ioVariables.begin(), parent->ioVariables.end(), this);
	if (p != parent->ioVariables.end())
		parent->ioVariables.erase(p);
}

void NodeNetworkVariable::Init()
{
	name = "NodeNetworkVariable";
	title = "Network Variable";
	minSpace = v2(100.0f, 40.0f);
	parent->ioVariables.push_back(this);
}

void NodeNetworkVariable::IO()
{
	if (isOutput)
		DefaultInput("out", &b, &i, &f, &c, &s, nodeType);
	else
		DefaultOutput("in", &b, &i, &f, &c, &s, nodeType);
}

void NodeNetworkVariable::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	title = id;
	for (int i = 0; i < 5; i++)
	{
		if ((int)nodeType == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), v4(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), v4(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool NodeNetworkVariable::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);

	if (nodeType != (NodeType)tcp)
	{
		if (isOutput)
			Disconnect(0);
		else
			DisconnectOutput(0);
			
	}
	nodeType = (NodeType)tcp;
	return true;
}

void NodeNetworkVariable::Load(JSONType& data)
{
	isOutput = data.obj["isO"].b;
	nodeType = (NodeType)data.obj["type"].i;
	id = data.obj["id"].s;
}

JSONType NodeNetworkVariable::Save()
{
	return JSONType({
		{ "isO", isOutput },
		{ "type", (long)nodeType },
		{ "id", id }
	});
}
