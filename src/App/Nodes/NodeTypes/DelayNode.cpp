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
	FloatInput("delay", &time, 0.0005f, 1.0f, true, false);
	if (delayType == DelayType::PingPong)
		FloatInput("stereonity", &stereoWideness, -1.0f, 1.0f, true, true);
}

void DelayNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	EnsureQueueSize(0);
	int skip = lodOn ? 4 : 1;
	for (int i = 0; i < 127; i += skip)
	{
		v2 va = queueLerp((float)i / 128.0f * queueSize[0], 0);
		v2 vb = queueLerp((float)(i + skip) / 128.0f * queueSize[0], 0);
		dl->Line(topLeft + v2((float)i, 50.0f + va.x * 10.0f), topLeft + v2((float)(i + skip), 50.0f + vb.x * 10.0f), v4(1.0f, 0.0f, 0.0f, 0.5f));
		dl->Line(topLeft + v2((float)i, 50.0f + va.y * 10.0f), topLeft + v2((float)(i + skip), 50.0f + vb.y * 10.0f), v4(0.0f, 1.0f, 0.0f, 0.5f));
	}

	for (int i = 0; i < 2; i++)
	{
		if ((int)delayType == i)
			dl->RectFilled(topLeft + v2(i * 64.0f, 0.0f), topLeft + v2(i * 64.0f + 64.0f, 20.0f), v4(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 64.0f, 0.0f), topLeft + v2(i * 64.0f + 64.0f, 20.0f), v4(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool DelayNode::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0 || info.pos.y >= 40.0f) 
		return false;

	int tcp = (int)(info.pos.x / 64.0f);
	delayType = (DelayType)tcp;
	return true;
}

void DelayNode::Work(int id)
{
	EnsureQueueSize(id);
	float wideness = stereoWideness * 0.5f + 0.5f;
	if (delayType == DelayType::Mono)
		for (int i = 0; i < ichannel.bufferSize; i++)
		{
			ochannel.data[i].x = queue[id][queuePointer[id]].x * mix + ichannel.data[i].x * (1.0f - mix);
			ochannel.data[i].y = queue[id][queuePointer[id]].y * mix + ichannel.data[i].y * (1.0f - mix);
			queue[id][queuePointer[id]] = queue[id][queuePointer[id]] * feedback + ichannel.data[i];
			queuePointer[id]++;
			queuePointer[id] %= queueSize[id];
		}
	else
		for (int i = 0; i < ichannel.bufferSize; i++)
		{
			// need to take wideness into account somehow
			v2 lr = v2(
				queue[id][queuePointer[id]].x, 
				queue[id][(queuePointer[id] + queueSize[id] / 2) % queueSize[id]].y
			);
			lr = v2(
				lr.x * wideness + lr.y * (1.0f - wideness),
				lr.y * wideness + lr.x * (1.0f - wideness)
			);

			ochannel.data[i] = lr * mix + ichannel.data[i] * (1.0f - mix);
			
			// update queue after setting output data so incoming signal is not immediately delayed
			queue[id][queuePointer[id]] = queue[id][queuePointer[id]] * feedback + ichannel.data[i];
			queuePointer[id]++;
			queuePointer[id] %= queueSize[id];
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

v2 DelayNode::queueLerp(float index, int id) const
{
	float lv = fmodf(index, 1.0f);
	return queue[id][(int)index % queueSize[id]] * lv + queue[id][(int)(index + 1.0f) % queueSize[id]] * (1.0f - lv);
}

void DelayNode::EnsureQueueSize(int id)
{
	queueSize[id] = (int)(time * ichannel.sampleRate * (delayType == DelayType::PingPong ? 2.0f : 1.0f));
	size_t s = queue[id].size();

	if (queuePointer[id] >= queueSize[id])
		queuePointer[id] = 0;

	if (queueSize[id] > s)
	{
		for (int i = 0; i < queueSize[id] - s; i++)
			queue[id].push_back(v2());
	}
	else if (queueSize[id] < s)
	{
		for (int i = 0; i < s - queueSize[id]; i++)
			queue[id].pop_back();
	}
	
}