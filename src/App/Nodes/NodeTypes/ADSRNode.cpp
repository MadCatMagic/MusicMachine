#include "App/Nodes/NodeTypes/ADSRNode.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void ADSRNode::Init()
{
	name = "ADSRNode";
	title = "ADSR";
	minSpace = v2(100.0f, 50.0f);
}

void ADSRNode::IO()
{
	SequencerInput("sequence", &isequencer);
	FloatInput("attack", &attack, 0.001f, 0.2f, true, false);
	FloatInput("decay", &decay, 0.001f, 1.0f, true, false);
	FloatInput("sustain", &sustain, 0.0f, 1.0f, true, true);
	FloatInput("release", &release, 0.005f, 1.0f, true, false);

	AudioOutput("adsr", &ochannel);
}

void ADSRNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	float k = minSpace.x / (attack + decay * 1.5f + release);

	std::vector<v2> curve = {
		topLeft + v2(0.0f, minSpace.y),
		topLeft + v2(k * attack, 0.0f),
		topLeft + v2(k * (attack + decay), minSpace.y * (1.0f - sustain)),
		topLeft + v2(k * (attack + decay * 1.5f), minSpace.y * (1.0f - sustain)),
		topLeft + minSpace
	};
	dl->Lines(curve, v4(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);
}

void ADSRNode::Work(int id)
{
	if (isequencer.length.size() == 0)
		return;

	int freq = 0;
	int scounter = 0;

	// exponential decrease
	float releaseFactor = powf(0.005f, 1.0f / (release * ochannel.sampleRate));

	for (size_t i = 0; i < ochannel.bufferSize; i++)
	{
		if (scounter >= isequencer.length[freq])
		{
			scounter = 0;
			freq++;
			if (freq >= isequencer.length.size())
				return;
		}

		if (isequencer.pitch[freq] != 0.0f)
		{
			lastSample[id] = adsr(scounter + isequencer.cumulativeSamples[freq], lastSample[id]);
			ochannel.data[i] = lastSample[id];
		}
		else
		{
			lastSample[id] *= releaseFactor;
			ochannel.data[i] = lastSample[id];
		}

		scounter++;
	}
}

void ADSRNode::Load(JSONType& data)
{
	attack = (float)data.obj["attack"].f;
	decay = (float)data.obj["decay"].f;
	sustain = (float)data.obj["sustain"].f;
	release = (float)data.obj["release"].f;
}

JSONType ADSRNode::Save()
{
	return JSONType({
		{ "attack", (double)attack },
		{ "decay", (double)decay },
		{ "sustain", (double)sustain },
		{ "release", (double)release }
	});
}

float ADSRNode::adsr(int sample, float last) const
{
	float t = (float)sample / ochannel.sampleRate;
	if (t <= attack)
	{
		float x = t / attack;
		// waits so there is no jumping from last to 0 on new attack
		if (last > x)
			return last;
		return x;
	}
	else if (t <= attack + decay)
		return 1.0f - ((t - attack) / decay) * (1.0f - sustain);
	else
		return sustain;
	return 0.0f;
}