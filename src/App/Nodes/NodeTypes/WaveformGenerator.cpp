#include "App/Nodes/NodeTypes/WaveformGenerator.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void WaveformGenerator::Init()
{
	// generate waveformData
	if (waveformData.size() == 0)
	{
		for (int j = 0; j < 4; j++)
		{
			std::vector<float> dump = std::vector<float>(1024);
			for (int i = 0; i < 1024; i++)
				dump[i] = GetValue((float)i / 1024.0f, j);
			waveformData.push_back(dump);
		}
	}

	name = "WaveformGenerator";
	title = "Waveform Generator";
	minSpace = v2(100.0f, 50.0f);
}

void WaveformGenerator::IO()
{
	AudioOutput("aout", &c);
	SequencerInput("sequence", &seq);
	FloatInput("shape", &shape, 0.0f, 3.0f, true, true);
}

void WaveformGenerator::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	int skip = lodOn ? 2 : 1;
	for (float i = 0.0f; i < 31.0f; i += (float)skip)
		dl->Line(
			topLeft + v2(
				i / 32.0f * 100.0f,
				25.0f - 25.0f * Bilinear(i / 32.0f)
			),
			topLeft + v2(
				(i + (float)skip) / 32.0f * 100.0f,
				25.0f - 25.0f * Bilinear((i + (float)skip) / 32.0f)
			),
			ImColor(0.0f, 1.0f, 1.0f)
		);
}

void WaveformGenerator::Work(int id)
{
	if (seq.length.size() == 0)
		return;

	int frequencyIndex = 0;
	size_t sampleCounter = 0;

	if (seq.cumulativeSamples[0] < AudioChannel::bufferSize || ( seq.pitch[0] != freq[id] && seq.pitch[0] != 0.0f))
	{
		kv[id] = 0.0f;
		freq[id] = seq.pitch[0];
		vel[id] = seq.velocity[0];
	}

	float samplesPerCycle = (AudioChannel::sampleRate / freq[id]);
	float kvIncrement = 1.0f / samplesPerCycle;

	for (size_t i = 0; i < c.bufferSize; i++)
	{
		if (sampleCounter >= seq.length[frequencyIndex])
		{
			sampleCounter = 0;
			frequencyIndex++;
			if (frequencyIndex >= seq.length.size())
				return;

			if (seq.cumulativeSamples[frequencyIndex] == 0 || ( seq.pitch[frequencyIndex] != freq[id] && seq.pitch[frequencyIndex] != 0.0f))
			{
				kv[id] = 0.0f;
				freq[id] = seq.pitch[frequencyIndex];
				vel[id] = seq.velocity[frequencyIndex];
			}

			samplesPerCycle = (AudioChannel::sampleRate / freq[id]);
			kvIncrement = 1.0f / samplesPerCycle;
		}

		if (freq[id] != 0.0f)
		{
			kv[id] += kvIncrement;
			if (kv[id] >= 1.0f) kv[id] -= 1.0f;
			c.data[i] = Bilinear(kv[id]) * vel[id];
		}

		sampleCounter++;
	}
}

void WaveformGenerator::Load(JSONType& data)
{
	shape = (float)data.obj["shape"].f;
}

JSONType WaveformGenerator::Save()
{
	auto map = std::unordered_map<std::string, JSONType>();
	map["shape"] = (double)shape;
	return map;
}

// phase is from 0 to 1
float WaveformGenerator::GetValue(float phase, int shape) const
{
	// bilinear interpolation again

	switch (shape)
	{
	case 0: return sinf(phase * 2.0f * PI);
	// downsaw
	case 3: return 1.0f - 2.0f * phase;
	case 2: return (phase < 0.5f) ? 1.0f : -1.0f;
	case 1:
		if (phase < 0.25f) return phase * 4.0f;
		else if (phase < 0.75f) return 2.0f - phase * 4.0f;
		else return 4.0f * phase - 4.0f;
	}
	return 0.0f;
}

// phase is still from 0 to 1
float WaveformGenerator::Bilinear(float phase) const
{
	int i1 = (int)(phase * 1024.0f) % 1024;
	int i2 = (i1 + 1) % 1024;
	float lerp = phase * 1024.0f - floorf(phase * 1024.0f);

	int wave1 = (int)(shape);
	int wave2 = (wave1 + 1) % 4;
	float wavelerp = shape - floorf(shape);

	return 
		(waveformData[wave1][i1] * (1.0f - lerp) + waveformData[wave1][i2] * lerp) * (1.0f - wavelerp) + 
		(waveformData[wave2][i1] * (1.0f - lerp) + waveformData[wave2][i2] * lerp) * wavelerp;
}

std::vector<std::vector<float>> WaveformGenerator::waveformData = {};