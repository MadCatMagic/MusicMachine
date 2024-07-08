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
	name = "AudioOutputNode";
	title = "Audio Output Node";
}

void AudioOutputNode::IO()
{
	AudioInput("ain", &c);
	FloatInput("volume", &volume, 0.0f, 1.0f, true);
}

AudioChannel* AudioOutputNode::Result()
{
	return &c;
}

void AudioOutputNode::Work()
{
	for (size_t i = 0; i < c.bufferSize; i++)
		c.data[i] *= volume;
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

	EnsureDataSize();
}

void SequencerNode::IO()
{
	SequencerOutput("sequence", &seq);
	FloatInput("bpm", &bpm, 10.0f, 600.0f, true);
	IntInput("length", &horizWidth, 4, 32, true);
	IntInput("height", &vertWidth, 12, 24, true);
}

#include "Engine/DrawList.h"
void SequencerNode::Render(const v2& topLeft, DrawList* dl)
{
	EnsureDataSize();
	// STUFF
	for (int i = 0; i < horizWidth; i++)
		for (int j = 0; j < vertWidth; j++)
			dl->Rect(topLeft + v2(i, j) * cellSize, topLeft + v2(i + 1.0f, j + 1.0f) * cellSize, ImColor(1.0f, (float)i / horizWidth, (float)j / vertWidth, 1.0f));

	for (int i = 0; i < horizWidth; i++)
	{
		if (data[i].first != -1)
			dl->RectFilled(topLeft + v2(i, vertWidth - 1 - data[i].first) * cellSize, topLeft + v2(i, vertWidth - 1 - data[i].first) * cellSize + cellSize, ImColor(0.7f, data[i].second, 0.2f, 1.0f));
	}
	dl->RectFilled(topLeft + v2(currentI * cellSize, 0.0f), topLeft + v2(currentI * cellSize + cellSize, cellSize * vertWidth), ImColor(1.0f, 0.2f, 0.1f, 0.4f));
}

bool SequencerNode::OnClick(const v2& clickPosition)
{
	v2i p = v2i(clickPosition / cellSize);
	p.y = vertWidth - 1 - p.y;
	if (data[p.x].first != p.y)
		data[p.x] = std::make_pair(p.y, 0.7f);
	else
		data[p.x] = std::make_pair(-1, 0.0f);
	return true;
}

void SequencerNode::Work()
{
	EnsureDataSize();

	currentI = (int)(AudioChannel::t * (bpm / 60.0f)) % horizWidth;

	float pitchLength = (float)AudioChannel::sampleRate / (bpm / 60.0f);

	seq = PitchSequencer();

	float fakeTime = AudioChannel::t * (bpm / 60.0f);

	int currLength = 0;
	int io = 0;
	while (currLength < AudioChannel::bufferSize)
	{
		float p = GetPitch((currentI + io) % horizWidth);

		// number of samples the sound should play for *THIS* buffer
		float exactI = fakeTime - (float)(int)fakeTime;
		if (io > 0)
			exactI = 0.0f;
		int t = (int)((1.0f - exactI) * pitchLength);
		t = (t + currLength) >= AudioChannel::bufferSize ? AudioChannel::bufferSize - currLength : t;

		// send pitch
		seq.pitch.push_back(p);
		seq.length.push_back(t);
		seq.velocity.push_back(data[(currentI + io) % horizWidth].second);
		currLength += t;

		io++;
	}

}

float SequencerNode::GetPitch(int i)
{
	const float mul = powf(2.0f, 1.0f / 12.0f);
	float b = 220.0f;
	if (data[i].first == -1)
		return 0.0f;

	for (int j = 0; j < data[i].first; j++)
		b *= mul;

	return b;
}

void SequencerNode::EnsureDataSize()
{
	minSpace = v2(horizWidth, vertWidth) * cellSize;
	size_t s = data.size();

	if (s == horizWidth)
		return;

	if (s < horizWidth)
		for (int i = 0; i < horizWidth - s; i++)
			data.push_back({ -1, 0.0f });
	else
		for (int i = 0; i < s - horizWidth; i++)
			data.erase(data.end() - 1);
}