#include "App/Nodes/NodeTypes/AudioOutputNode.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void AudioOutputNode::Init()
{
	minSpace = v2(200.0f, 50.0f);
	name = "AudioOutputNode";
	title = "Audio Output Node";
}

void AudioOutputNode::IO()
{
	AudioInput("ain", &c);
	FloatInput("volume", &volume, 0.0f, 1.0f, true, true, Node::FloatDisplayType::Db);
	IntInput("viewScale", &previousDataDivider, 0, 5, true, true);
}

AudioChannel* AudioOutputNode::Result()
{
	return &c;
}

void AudioOutputNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	int bufferSize = 8192 / (1 << previousDataDivider);
	int skip = (lodOn ? 128 : 32) / (1 << previousDataDivider);
	const float bw = 200.0f / (float)bufferSize;
	for (int i = 0; i < (bufferSize - skip); i += skip)
	{
		dl->Line(
			topLeft + v2(bw * i, 25.0f + previousData[i].x * 25.0f),
			topLeft + v2(bw * i + bw * skip, 25.0f + previousData[i + skip].x * 25.0f),
			ImColor(1.0f, 0.0f, 0.0f, 0.5f)
		);
		dl->Line(
			topLeft + v2(bw * i, 25.0f + previousData[i].y * 25.0f),
			topLeft + v2(bw * i + bw * skip, 25.0f + previousData[i + skip].y * 25.0f),
			ImColor(0.0f, 1.0f, 0.0f, 0.5f)
		);
	}
}

void AudioOutputNode::Work(int id)
{
	previousDataP %= 8192 / (1 << previousDataDivider);
	for (size_t i = 0; i < c.bufferSize; i++)
	{
		previousData[previousDataP++] = -c.data[i];
		previousDataP %= 8192 / (1 << previousDataDivider);
		c.data[i] *= volume;
	}
}

void AudioOutputNode::Load(JSONType& data)
{
	volume = (float)data.f;
}

JSONType AudioOutputNode::Save()
{
	return JSONType((double)volume);
}