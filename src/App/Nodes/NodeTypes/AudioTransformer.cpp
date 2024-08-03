#include "App/Nodes/NodeTypes/AudioTransformer.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void MixNode::Init()
{
	name = "MixNode";
	title = "Mix";
	minSpace = v2(20.0f * numTypes, 20.0f);
}

void MixNode::IO()
{
	AudioInput("A source", &ic1);
	AudioInput("B source", &ic2);
	if (type == TransformationType::WeightedAdd)
	{
		FloatInput("a volume", &weight1, 0.0f, 1.5f, true, true, FloatDisplayType::Db);
		FloatInput("b volume", &weight2, 0.0f, 1.5f, true, true, FloatDisplayType::Db);
	}
	AudioOutput("O", &oc);
}

void MixNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < numTypes; i++)
	{
		if ((int)type == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
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
		}
	};

	for (int j = 0; j < 2; j++)
	{
		for (size_t i = 0; i < lineData[j].size() / 2; i++)
			dl->Line(
				topLeft + v2(10.0f + j * 20.0f, 10.0f) + lineData[j][i * 2] * 10.0f,
				topLeft + v2(10.0f + j * 20.0f, 10.0f) + lineData[j][i * 2 + 1] * 10.0f,
				ImColor(1.0f, 1.0f, 1.0f),
				1.0f / dl->scaleFactor
			);
	}
}

bool MixNode::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);
	type = (TransformationType)tcp;
	return true;
}

void MixNode::Work()
{
	if (type == TransformationType::WeightedAdd)
	{
		for (size_t i = 0; i < oc.bufferSize; i++)
			oc.data[i] = ic1.data[i] * weight1 + ic2.data[i] * weight2;
	}
	else if (type == TransformationType::Multiply)
	{
		for (size_t i = 0; i < oc.bufferSize; i++)
			oc.data[i] = ic1.data[i].scale(ic2.data[i]);
	}
}

void MixNode::Load(JSONType& data)
{
	type = (TransformationType)data.obj["type"].i;
	weight1 = (float)data.obj["w1"].f;
	weight2 = (float)data.obj["w2"].f;
}

JSONType MixNode::Save()
{
	return JSONType({
		{ "type", (long)type },
		{ "w1", (double)weight1 },
		{ "w2", (double)weight2 }
		});
}