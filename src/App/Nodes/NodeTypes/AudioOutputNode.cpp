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
}

AudioChannel* AudioOutputNode::Result()
{
	return &c;
}

void AudioOutputNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	int skip = lodOn ? 4 : 1;
	const float bw = 200.0f / 256.0f;
	for (int i = 0; i < 255; i += skip)
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

void AudioOutputNode::Work()
{
	for (size_t i = 0; i < c.bufferSize; i++)
	{
		if (i % 128 == 0)
		{
			previousData[previousDataP++] = -c.data[i];
			previousDataP %= 256;
		}
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