#include "App/Nodes/NodeTypes/AudioTransformer.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void MixNode::Init()
{
	name = "MixNode";
	title = "Mix";
	minSpace = v2(20.0f * numTypes, 40.0f);
}

void MixNode::IO()
{
	EnsureCorrectChannelNum();
	for (int i = 0; i < numChannels; i++)
	{
		AudioInput("Source " + std::to_string(i), &ichannels[i]);
		if (type == TransformationType::WeightedAdd)
			FloatInput("Volume " + std::to_string(i), &weights[i], 0.0f, 1.5f, true, true, FloatDisplayType::Db);
	}
	AudioOutput("Out", &ochannel);
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
	if (info.pos.y <= 20.0f)
		type = (TransformationType)tcp;
	else
		numChannels += tcp * 2 - 1;
	return true;
}

void MixNode::Work(int id)
{
	EnsureCorrectChannelNum();

	if (type == TransformationType::WeightedAdd)
	{
		for (size_t i = 0; i < ochannel.bufferSize; i++)
			for (int j = 0; j < numChannels; j++)
				ochannel.data[i] += ichannels[j].data[i] * weights[j];
	}
	else if (type == TransformationType::Multiply)
	{
		for (size_t i = 0; i < ochannel.bufferSize; i++)
		{
			ochannel.data[i] = 1.0f;
			for (int j = 0; j < numChannels; j++)
				ochannel.data[i] = ochannel.data[i].scale(ichannels[j].data[i]);
		}
	}
}

void MixNode::Load(JSONType& data)
{
	type = (TransformationType)data.obj["type"].i;
	numChannels = (int)data.obj["numChannels"].i;

	for (const JSONType& i : data.obj["weights"].arr)
		weights.push_back((float)i.f);
}

JSONType MixNode::Save()
{
	std::vector<JSONType> weightArr;
	for (float w : weights)
		weightArr.push_back(w);
	return JSONType({
		{ "type", (long)type },
		{ "numChannels", (long)numChannels },
		{ "weights", weightArr }
	});
}

void MixNode::EnsureCorrectChannelNum()
{
	numChannels = clamp(numChannels, 2, 16);

	int s = (int)ichannels.size();
	if (numChannels > s)
	{
		for (int i = 0; i < numChannels - s; i++)
		{
			ichannels.push_back(AudioChannel());
			weights.push_back(1.0f);
		}
	}
	else if (numChannels < s)
	{
		for (int i = 0; i < s - numChannels; i++)
		{
			ichannels.pop_back();
			weights.pop_back();
		}
	}
}
