#include "App/Nodes/NodeTypes/DelayNode.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void DelayNode::Init()
{
	name = "DelayNode";
	title = "Delay";
	minSpace = v2(128.0f, 80.0f);
}

void DelayNode::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("feedback", &feedback, 0.0f, 1.0f, true, true);
	FloatInput("mix", &mix, 0.0f, 1.0f, true, true);
	FloatInput("delay", &time, 0.1f, 1.0f, true, false);
	if (delayType == DelayType::PingPong)
		FloatInput("stereonity", &stereoWideness, -1.0f, 1.0f, true, true);
}

void DelayNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	EnsureQueueSize();
	int skip = lodOn ? 4 : 1;
	for (int i = 0; i < 127; i += skip)
	{
		v2 va = queueLerp((float)i / 128.0f * queueSize);
		v2 vb = queueLerp((float)(i + skip) / 128.0f * queueSize);
		dl->Line(topLeft + v2((float)i, 50.0f + va.x * 10.0f), topLeft + v2((float)(i + skip), 50.0f + vb.x * 10.0f), ImColor(1.0f, 0.0f, 0.0f, 0.5f));
		dl->Line(topLeft + v2((float)i, 50.0f + va.y * 10.0f), topLeft + v2((float)(i + skip), 50.0f + vb.y * 10.0f), ImColor(0.0f, 1.0f, 0.0f, 0.5f));
	}

	for (int i = 0; i < 2; i++)
	{
		if ((int)delayType == i)
			dl->RectFilled(topLeft + v2(i * 64.0f, 0.0f), topLeft + v2(i * 64.0f + 64.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 64.0f, 0.0f), topLeft + v2(i * 64.0f + 64.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool DelayNode::OnClick(const v2& clickPosition)
{
	if (clickPosition.y >= 40.0f) return false;
	int tcp = (int)(clickPosition.x / 64.0f);
	delayType = (DelayType)tcp;
	return true;
}

void DelayNode::Work()
{
	EnsureQueueSize();
	float wideness = stereoWideness * 0.5f + 0.5f;
	if (delayType == DelayType::Mono)
		for (int i = 0; i < ichannel.bufferSize; i++)
		{
			queue[queuePointer] = queue[queuePointer] * feedback + ichannel.data[i];
			ochannel.data[i].x = queue[queuePointer].x * mix + ichannel.data[i].x * (1.0f - mix);
			ochannel.data[i].y = queue[queuePointer].y * mix + ichannel.data[i].y * (1.0f - mix);
			queuePointer++;
			queuePointer %= queueSize;
		}
	else
		for (int i = 0; i < ichannel.bufferSize; i++)
		{
			queue[queuePointer] = queue[queuePointer] * feedback + ichannel.data[i];
			// need to take wideness into account somehow
			v2 lr = v2(
				queue[queuePointer].x, 
				queue[(queuePointer + queueSize / 2) % queueSize].y
			);
			lr = v2(
				lr.x * wideness + lr.y * (1.0f - wideness),
				lr.y * wideness + lr.x * (1.0f - wideness)
			);

			ochannel.data[i] = lr * mix + ichannel.data[i] * (1.0f - mix);
			queuePointer++;
			queuePointer %= queueSize;
		}
}

void DelayNode::Load(JSONType& data)
{
	feedback = (float)data.obj["feedback"].f;
	mix = (float)data.obj["mix"].f;
	time = (float)data.obj["time"].f;
	delayType = (DelayType)data.obj["type"].i;
	stereoWideness = (float)data.obj["stereo"].f;
}

JSONType DelayNode::Save()
{
	return JSONType({
		{ "feedback", (double)feedback },
		{ "mix", (double)mix },
		{ "time", (double)time },
		{ "type", (long)delayType },
		{ "stereo", (long)stereoWideness }
	});
}

v2 DelayNode::queueLerp(float index) const
{
	float lv = fmodf(index, 1.0f);
	return queue[(int)index % queueSize] * lv + queue[(int)(index + 1.0f) % queueSize] * (1.0f - lv);
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