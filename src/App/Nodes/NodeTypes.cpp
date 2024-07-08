#include "App/Nodes/NodeTypes.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

/*void MathsNode::Load(JSONType& data)
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
}*/

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

		if (seq.pitch[freq] != 0.0f)
		{
			kv += increment;
			if (kv >= 1.0f) kv -= 2.0f;
			c.data[i] = v2(kv, kv) * seq.velocity[freq];
		}

		scounter++;
	}
}

void AudioOutputNode::Init()
{
	minSpace = v2(200.0f, 50.0f);
	name = "AudioOutputNode";
	title = "Audio Output Node";
}

void AudioOutputNode::IO()
{
	AudioInput("ain", &c);
	FloatInput("volume", &volume, 0.0f, 1.0f, true, true);
}

AudioChannel* AudioOutputNode::Result()
{
	return &c;
}

void AudioOutputNode::Render(const v2& topLeft, DrawList* dl)
{
	const float bw = 200.0f / 1024.0f;
	dl->Line(topLeft + v2(0.0f, 25.0f), topLeft + v2(200.0f, 25.0f), ImColor(1.0f, 1.0f, 1.0f, 0.2f));
	for (int i = 0; i < 1023; i++)
	{
		dl->Line(
			topLeft + v2(bw * i, 25.0f + previousData[i].x * 25.0f), 
			topLeft + v2(bw * i + bw, 25.0f + previousData[i + 1].x * 25.0f),
			ImColor(1.0f, 0.0f, 0.0f, 0.5f)
		);
		dl->Line(
			topLeft + v2(bw * i, 25.0f + previousData[i].y * 25.0f),
			topLeft + v2(bw * i + bw, 25.0f + previousData[i + 1].y * 25.0f),
			ImColor(0.0f, 1.0f, 0.0f, 0.5f)
		);
	}
	//ImGui::PlotHistogram("previousSamplesLeft", astream.previousData, 1024, 0, 0, -1.0f, 1.0f, ImVec2(0.0f, 40.0f));
}

void AudioOutputNode::Work()
{
	for (size_t i = 0; i < c.bufferSize; i++)
	{
		if (i % 8 == 0)
		{
			previousData[previousDataP++] = c.data[i];
			previousDataP %= 1024;
		}
		c.data[i] *= volume;
	}
}

void AudioTransformer::Init()
{
	name = "AudioAdder";
	title = "Audio Adder";
	minSpace = v2(20.0f * numTypes, 20.0f);
}

void AudioTransformer::IO()
{
	AudioInput("A", &ic1);
	AudioInput("B", &ic2);
	if (type == TransformationType::Lerp)
		FloatInput("k", &lerp, 0.0f, 1.0f, true, true);
	AudioOutput("O", &oc);
}

void AudioTransformer::Render(const v2& topLeft, DrawList* dl)
{
	for (int i = 0; i < numTypes; i++)
	{
		if ((int)type == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool AudioTransformer::OnClick(const v2& clickPosition)
{
	int tcp = (int)(clickPosition.x / 20.0f);
	type = (TransformationType)tcp;
	return true;
}

void AudioTransformer::Work()
{
	if (type == TransformationType::Lerp)
	{
		for (size_t i = 0; i < oc.bufferSize; i++)
			oc.data[i] = ic1.data[i] * lerp + ic2.data[i] * (1.0f - lerp);
	}
	else if (type == TransformationType::Multiply)
	{
		for (size_t i = 0; i < oc.bufferSize; i++)
			oc.data[i] = ic1.data[i].scale(ic2.data[i]);
	}
}

void ADSRNode::Init()
{
	name = "ADSRNode";
	title = "ADSR";
}

// TODOODODODODODOD
void ADSRNode::IO()
{
	SequencerInput("sequence", &isequencer);
	FloatInput("attack", &attack, 0.0f, 0.2f, true, false);
	FloatInput("decay", &decay, 0.0f, 1.0f, true, false);
	FloatInput("sustain", &sustain, 0.0f, 1.0f, true, true);
	FloatInput("release", &sustain, 0.0f, 1.0f, true, false);
	
	AudioOutput("adsr", &ochannel);
}

void ADSRNode::Render(const v2& topLeft, DrawList* dl)
{
}

bool ADSRNode::OnClick(const v2& clickPosition)
{
	return false;
}

void ADSRNode::Work()
{
}
