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
	SequencerInput("sequence", &seq);
}

void SawWave::Work()
{
	if (seq.length.size() == 0)
		return;


	int freq = 0;
	size_t scounter = 0;

	float samplesPerCycle = (c.sampleRate / seq.pitch[freq]);
	float increment = 1.0f / samplesPerCycle;

	for (size_t i = 0; i < c.bufferSize; i++)
	{
		if (scounter >= seq.length[freq])
		{
			scounter = 0;
			freq++;
			if (freq >= seq.length.size())
				return;

			samplesPerCycle = (c.sampleRate / seq.pitch[freq]);
			increment = 1.0f / samplesPerCycle;
		}

		kv += increment;
		if (kv >= 1.0f) kv -= 2.0f;
		c.data[i] = v2(kv, kv) * seq.velocity[freq];

		scounter++;
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

void AudioAdder::Init()
{
	name = "AudioAdder";
	title = "Audio Adder";
}

void AudioAdder::IO()
{
	AudioInput("A", &ic1);
	AudioInput("B", &ic2);
	FloatInput("k", &lerp, 0.0f, 1.0f, true);
	AudioOutput("O", &oc);
}

void AudioAdder::Load(JSONType& data)
{
	lerp = (float)data.f;
}

JSONType AudioAdder::Save()
{
	return JSONType((double)lerp);
}

void AudioAdder::Work()
{
	for (size_t i = 0; i < oc.bufferSize; i++)
	{
		oc.data[i] = ic1.data[i] * lerp + ic2.data[i] * (1.0f - lerp);
	}
}

void SequencerNode::Init()
{
	name = "SequencerNode";
	title = "Sequencer Node";
}

void SequencerNode::IO()
{
	SequencerOutput("sequence", &seq);
}

void SequencerNode::Work()
{
	seq.length = {
		1024
	};
	seq.pitch = {
		440.0f
	};
	seq.velocity = {
		0.5f
	};
}
