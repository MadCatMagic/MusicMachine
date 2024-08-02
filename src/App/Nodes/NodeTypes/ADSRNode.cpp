#include "App/Nodes/NodeTypes/ADSRNode.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void ADSRNode::Init()
{
	name = "ADSRNode";
	title = "ADSR";
}

// TODOODODODODODOD
void ADSRNode::IO()
{
	SequencerInput("sequence", &isequencer);
	FloatInput("attack", &attack, 0.001f, 0.2f, true, false);
	FloatInput("decay", &decay, 0.0f, 1.0f, true, false);
	FloatInput("sustain", &sustain, 0.0f, 1.0f, true, true);
	FloatInput("release", &release, 0.005f, 1.0f, true, false);

	AudioOutput("adsr", &ochannel);
}

void ADSRNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
}

void ADSRNode::Work()
{
	if (isequencer.length.size() == 0)
		return;

	int freq = 0;
	int scounter = 0;

	// amazingly works pretty well
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
			lastSample = adsr(scounter + isequencer.cumSamples[freq]);
			ochannel.data[i] = lastSample;
		}
		else
		{
			lastSample *= releaseFactor;
			ochannel.data[i] = lastSample;
		}

		scounter++;
	}
}

float ADSRNode::adsr(int sample) const
{
	float t = (float)sample / ochannel.sampleRate;
	if (t <= attack)
		return t / attack;
	else if (t <= attack + decay)
		return 1.0f - ((t - attack) / decay) * (1.0f - sustain);
	else
		return sustain;
	return 0.0f;
}