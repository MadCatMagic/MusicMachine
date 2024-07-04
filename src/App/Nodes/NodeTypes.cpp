#include "App/Nodes/NodeTypes.h"
#include "Engine/Console.h"

void MathsNode::Init()
{
	name = "MathsNode";
	title = "Maths innit";
	minSpace = v2(30, 40);
}

void MathsNode::IO()
{
	FloatInput("Input A", &a, 0.0f, 2.0f);
	FloatInput("Input B", &b);
	IntInput("Integer brugh", &c);
	BoolInput("Freak out", &crazy);
	FloatOutput("Result", &result);
	IntOutput("Result rounded", &resultRounded);

	AudioInput("ain", &ac);
	AudioOutput("aout", &ac);
}

void MathsNode::Load(JSONType& data)
{
	a = (float)data.obj["a"].f;
	b = (float)data.obj["b"].f;
	c = (int)data.obj["c"].i;
	crazy = data.obj["crazy"].b;
}

JSONType MathsNode::Save()
{
	return JSONType({
		{ "a", (double)a },
		{ "b", (double)b },
		{ "c", (long)c },
		{ "crazy", crazy }
	});
}

void SawWave::Init()
{
	name = "SawWave";
	title = "Saw Wave";
}

void SawWave::IO()
{
	AudioOutput("aout", &c);
	FloatInput("freq", &freq, 20.0f, 500.0f);
}

void SawWave::Load(JSONType& data)
{
	freq = (float)data.f;
}

JSONType SawWave::Save()
{
	return (double)freq;
}

void SawWave::Work()
{
	float samplesPerCycle = (c.sampleRate / freq);
	float increment = 1.0f / samplesPerCycle;

	for (size_t i = 0; i < c.bufferSize; i++)
	{
		kv += increment;
		if (kv >= 1.0f) kv -= 2.0f;
		c.data[i] = v2(kv, -kv);
	}
}

void AudioOutputNode::Init()
{
	name = "AudioOutputNode";
	title = "Audio Output Node";
}

void AudioOutputNode::IO()
{
	AudioInput("ain", &c);
}

AudioChannel* AudioOutputNode::Result()
{
	return &c;
}
