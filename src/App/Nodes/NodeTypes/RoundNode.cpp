#include "App/Nodes/NodeTypes/RoundNode.h"
#include "Engine/DrawList.h"

void RoundNode::Init()
{
	title = "Rounder";
	name = "RoundNode";
	minSpace = v2(100.0f, 10.0f);
}

void RoundNode::IO()
{
	if (floatInput)
	{
		FloatInput("f in", &finput);
		IntOutput("i out", &ioutput);
	}
	else
	{
		IntInput("i in", &iinput);
		FloatOutput("f out", &foutput);
	}
}

void RoundNode::Work(int id)
{
	foutput = (float)iinput;
	ioutput = (int)lroundf(finput);
}

void RoundNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	if (floatInput)
		dl->RectFilled(topLeft, topLeft + v2(100.0f, 10.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
	else
		dl->RectFilled(topLeft, topLeft + v2(100.0f, 10.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
}

bool RoundNode::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	floatInput = !floatInput;
	return true;
}

void RoundNode::Load(JSONType& data)
{
	floatInput = data.b;
}

JSONType RoundNode::Save()
{
	return JSONType(floatInput);
}
