#include "App/Nodes/NodeTypes/WaveformGenerator.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void WaveformGenerator::Init()
{
	name = "WaveformGenerator";
	title = "Waveform Generator";
	minSpace = v2(80.0f, 20.0f);
}

void WaveformGenerator::IO()
{
	AudioOutput("aout", &c);
	SequencerInput("sequence", &seq);
}

void WaveformGenerator::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < 4; i++)
	{
		if ((int)shape == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}

	if (lodOn)
		return;

	// Sine, Saw, Triangle, Square
	const std::vector<std::vector<v2>> lineData = {
		{ v2(-0.7f, 0.0f), v2(-0.525f, -0.495f), v2(-0.4375f, -0.6467f), v2(-0.35f, -0.7f), v2(-0.2625f, -0.6467f), v2(-0.175f, -0.495f), v2(0.0f, 0.0f), v2(0.175f, 0.495f), v2(0.2625f, 0.6467f), v2(0.35f, 0.7f), v2(0.4375f, 0.6467f), v2(0.525f, 0.495f), v2(0.7f, 0.0f) },
		{ v2(-0.7f, 0.0f), v2(-0.7f, -0.7f), v2(0.7f, 0.7f), v2(0.7f, 0.0f) },
		{ v2(-0.7f, 0.0f), v2(-0.35f, -0.7f), v2(0.35f, 0.7f), v2(0.7f, 0.0f) },
		{ v2(-0.7f, 0.0f), v2(-0.7f, -0.7f), v2(0.0f, -0.7f), v2(0.0f, 0.7f), v2(0.7f, 0.7f), v2(0.7f, 0.0f) }
	};

	for (int j = 0; j < 4; j++)
	{
		std::vector<v2> k;
		for (const v2& v : lineData[j])
			k.push_back(v * 10.0f + topLeft + v2(10.0f + j * 20.0f, 10.0f));
		dl->Lines(k, ImColor(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);
	}
}

bool WaveformGenerator::OnClick(const v2& clickPosition)
{
	int tcp = (int)(clickPosition.x / 20.0f);
	shape = (Shape)tcp;
	return true;
}

void WaveformGenerator::Work()
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
			if (kv >= 1.0f) kv -= 1.0f;
			c.data[i] = GetValue(kv) * seq.velocity[freq];
		}

		scounter++;
	}
}

// phase is from 0 to 1
float WaveformGenerator::GetValue(float phase) const
{
	switch (shape)
	{
	case Shape::Sine: return sinf(phase * 2.0f * PI);
	// downsaw
	case Shape::Saw: return 1.0f - 2.0f * phase;
	case Shape::Square: return (phase < 0.5f) ? 1.0f : -1.0f;
	case Shape::Triangle:
		if (phase < 0.25f) return phase * 4.0f;
		else if (phase < 0.75f) return 2.0f - phase * 4.0f;
		else return 4.0f * phase - 4.0f;
	}
}
