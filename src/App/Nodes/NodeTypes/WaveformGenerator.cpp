#include "App/Nodes/NodeTypes/WaveformGenerator.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void WaveformGenerator::Init()
{
	name = "WaveformGenerator";
	title = "Waveform Generator";
}

void WaveformGenerator::IO()
{
	AudioOutput("aout", &c);
	SequencerInput("sequence", &seq);
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
			if (kv >= 1.0f) kv -= 2.0f;
			c.data[i] = v2(kv, kv) * seq.velocity[freq];
		}

		scounter++;
	}
}