#include "App/Nodes/NodeTypes/DelayNode.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void DelayNode::Init()
{
	name = "DelayNode";
	title = "Delay";
	minSpace = v2(64.0f, 40.0f);
}

void DelayNode::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("feedback", &feedback, 0.0f, 1.0f, true, true);
	FloatInput("mix", &mix, 0.0f, 1.0f, true, true);
	FloatInput("delay", &time, 0.1f, 1.0f, true, false);
}

void DelayNode::Render(const v2& topLeft, DrawList* dl)
{
	EnsureQueueSize();
	for (int i = 0; i < 127; i++)
	{
		dl->Line(topLeft + v2((float)i / 128.0f * 64.0f, 10.0f + queue[i * skipLength].x * 10.0f), topLeft + v2((float)(i + 1) / 128.0f * 64.0f, 10.0f + queue[i * skipLength + skipLength].x * 10.0f), ImColor(1.0f, 0.0f, 0.0f, 0.5f));
		dl->Line(topLeft + v2((float)i / 128.0f * 64.0f, 10.0f + queue[i * skipLength].y * 10.0f), topLeft + v2((float)(i + 1) / 128.0f * 64.0f, 10.0f + queue[i * skipLength + skipLength].y * 10.0f), ImColor(0.0f, 1.0f, 0.0f, 0.5f));
	}
}

bool DelayNode::OnClick(const v2& clickPosition)
{
	int tcp = (int)(clickPosition.x / 20.0f);
	if (clickPosition.x >= 40.0f || clickPosition.y < 20.0f)
		return false;
	delayType = (DelayType)tcp;
	return true;
}

void DelayNode::Work()
{
	EnsureQueueSize();
	for (int i = 0; i < ichannel.bufferSize; i++)
	{
		queue[queuePointer] = queue[queuePointer] * feedback + ichannel.data[i];
		ochannel.data[i].x = queue[queuePointer].x * mix + ichannel.data[i].x * (1.0f - mix);
		ochannel.data[i].y = queue[(queuePointer + queueSize / 2) % queueSize].y * mix + ichannel.data[i].x * (1.0f - mix);
		queuePointer++;
		queuePointer %= queueSize;
	}
}

void DelayNode::Load(JSONType& data)
{
	feedback = (float)data.obj["feedback"].f;
	mix = (float)data.obj["mix"].f;
	time = (float)data.obj["time"].f;
}

JSONType DelayNode::Save()
{
	return JSONType({
		{ "feedback", (double)feedback },
		{ "mix", (double)mix },
		{ "time", (double)time }
		});
}

void DelayNode::EnsureQueueSize()
{
	if (time < 0.1f)
		time = 0.1f;
	queueSize = (int)(time * ichannel.sampleRate * (delayType == DelayType::PingPong ? 2.0f : 1.0f));
	skipLength = queueSize / 256;
	size_t s = queue.size();

	if (queuePointer >= queueSize)
		queuePointer = 0;

	if (queueSize > s)
	{
		for (int i = 0; i < queueSize - s; i++)
			queue.push_back(v2());
	}
	else if (queueSize < s)
	{
		for (int i = 0; i < s - queueSize; i++)
			queue.pop_back();
	}
}