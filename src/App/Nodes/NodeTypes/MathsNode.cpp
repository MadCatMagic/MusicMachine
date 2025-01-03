#include "App/Nodes/NodeTypes/MathsNode.h"
#include "Engine/DrawList.h"

void MathsNode::Init()
{
    name = "MathsNode";
    title = "Maths";
    minSpace = v2(80.0f, 20.0f);
}

void MathsNode::IO()
{
	FloatInput("input A", &inputA);
	FloatInput("input B", &inputB);
	FloatOutput("output", &output);
}

void MathsNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < 4; i++)
	{
		if ((int)op == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), v4(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), v4(0.1f, 0.2f, 0.5f, 0.3f));
	}

	if (lodOn)
		return;

	const std::vector<std::vector<v2>> lineData = {
		{
			v2(0.0f, -0.7f), v2(0.0f, 0.7f),
			v2(-0.7f, 0.0f), v2(0.7f, 0.0f)
		},
		{
			v2(-0.6f, -0.6f), v2(0.6f, 0.6f),
			v2(-0.6f, 0.6f), v2(0.6f, -0.6f)
		},
		{
			v2(-0.7f, 0.0f), v2(0.7f, 0.0f)
		},
		{
			v2(-0.7f, 0.0f), v2(0.7f, 0.0f),
			v2(0.0f, -0.7f), v2(0.0f, -0.6f),
			v2(0.0f, 0.6f), v2(0.0f, 0.7f)
		}
	};

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < lineData[j].size() / 2; i++)
			dl->Line(
				topLeft + v2(10.0f + j * 20.0f, 10.0f) + lineData[j][i * 2] * 10.0f,
				topLeft + v2(10.0f + j * 20.0f, 10.0f) + lineData[j][i * 2 + 1] * 10.0f,
				v4(1.0f, 1.0f, 1.0f),
				1.0f / dl->scaleFactor
			);
	}
}

bool MathsNode::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);
	op = (Operation)tcp;
	return true;
}

void MathsNode::Work(int id)
{
	switch (op)
	{
	case Operation::Add:
		output = inputA + inputB;
		return;

	case Operation::Multiply:
		output = inputA * inputB;
		return;

	case Operation::Subtract:
		output = inputA - inputB;
		return;

	case Operation::Divide:
		// just let division by zero be equal to zero
		// what can go wrong
		if (abs(inputB) < 0.00001f)
			output = 0.0f;
		else
			output = inputA / inputB;
		return;
	}
}

void MathsNode::Load(JSONType& data)
{
	inputA = (float)data.obj["a"].f;
	inputB = (float)data.obj["b"].f;
	op = (Operation)data.obj["op"].i;
}

JSONType MathsNode::Save()
{
	return JSONType({
		{ "a", (double)inputA },
		{ "b", (double)inputB },
		{ "op", (long)op },
	});
}
