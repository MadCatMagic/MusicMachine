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
	minSpace = v2(40.0f, 20.0f);
	parent->ioVariables.push_back(this);
}

void NodeNetworkVariable::IO()
{
	if (isOutput)
		AudioOutput("out", &channel);
	else
		AudioInput("in", &channel);
}

void NodeNetworkVariable::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < 2; i++)
	{
		if ((int)isOutput == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool NodeNetworkVariable::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);
	isOutput = (bool)tcp;
	return true;
}
