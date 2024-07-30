#include "App/Nodes/NodeTypes.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

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
	const float bw = 200.0f / 256.0f;
	for (int i = 0; i < 255; i++)
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
		if (i % 32 == 0)
		{
			previousData[previousDataP++] = c.data[i];
			previousDataP %= 256;
		}
		c.data[i] *= volume;
	}
}

void AudioOutputNode::Load(JSONType& data)
{
	volume = (float)data.f;
}

JSONType AudioOutputNode::Save()
{
	return JSONType((double)volume);
}

void AudioTransformer::Init()
{
	name = "AudioTransformer";
	title = "Audio Transformer";
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
	if (tcp == (int)(minSpace.x / 20.0f))
		return false;
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

void AudioTransformer::Load(JSONType& data)
{
	type = (TransformationType)data.obj["type"].i;
	lerp = (float)data.obj["lerp"].f;
}

JSONType AudioTransformer::Save()
{
	return JSONType({
		{ "type", (long)type },
		{ "lerp", (double)lerp }
	});
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
	if (isequencer.length.size() == 0)
		return;

	int freq = 0;
	size_t scounter = 0;

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
			ochannel.data[i] = adsr((int)scounter + isequencer.cumSamples[freq]);

		scounter++;
	}
}

float ADSRNode::adsr(int sample) const
{
	float t = (float)sample / ochannel.sampleRate;
	if (t <= attack)
		return t / attack;
	else if (t <= attack + decay)
		return 1.0f - ((t - attack) / decay) * sustain;
	else
		return sustain;
	return 0.0f;
}

void Distortion::Init()
{
	name = "Distortion";
	title = "Distortion";
}

void Distortion::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("pregain", &pregain, 0.0f, 1.0f, true, false);
	FloatInput("distortion", &distortion, 0.0f, 0.98f, true, true);
	FloatInput("mix", &mix, 0.0f, 1.0f, true, true);
}

void Distortion::Render(const v2& topLeft, DrawList* dl)
{
}

bool Distortion::OnClick(const v2& clickPosition)
{
	return false;
}

void Distortion::Work()
{
	for (size_t i = 0; i < ichannel.bufferSize; i++)
	{
		v2 pm = ichannel.data[i] * pregain;
		v2 v = v2(
			pm.x >= 0.0f ? powf(pm.x, 1.0f - distortion) : -powf(-pm.x, 1.0f - distortion),
			pm.y >= 0.0f ? powf(pm.y, 1.0f - distortion) : -powf(-pm.y, 1.0f - distortion)
		);
		ochannel.data[i] = v * mix + ichannel.data[i] * (1.0f - mix);
	}
}

void Distortion::Load(JSONType& data)
{
	pregain = (float)data.obj["pregain"].f;
	distortion = (float)data.obj["distortion"].f;
	mix = (float)data.obj["mix"].f;
}

JSONType Distortion::Save()
{
	return JSONType({
		{ "pregain", (double)pregain },
		{ "distortion", (double)distortion },
		{ "mix", (double)mix }
	});
}

void AudioFilter::Init()
{
	name = "AudioFilter";
	title = "Filter";
	minSpace = v2(60.0f, 20.0f);
}

void AudioFilter::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("cutoff", &cutoff, 0.0f, 0.99f, true, true);
	FloatInput("resonance", &resonance, 0.01f, 0.99f, true, true);
}

void AudioFilter::Render(const v2& topLeft, DrawList* dl)
{
	for (int i = 0; i < 3; i++)
	{
		if ((int)mode == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool AudioFilter::OnClick(const v2& clickPosition)
{
	int tcp = (int)(clickPosition.x / 20.0f);
	if (tcp == (int)(minSpace.x / 20.0f))
		return false;
	mode = (FilterType)tcp;
	return true;
}

void AudioFilter::Work()
{
	CalculateFeedbackAmount();

	if (isnan(buf0.x) || isnan(buf0.y) || isnan(buf1.x) || isnan(buf1.y))
	{
		buf0 = 0.0f; buf1 = 0.0f;
		Console::Log("Filter buffers turned to NaNs");
	}
	for (int i = 0; i < ichannel.bufferSize; i++)
	{
		buf0 += (ichannel.data[i] - buf0 + (buf0 - buf1) * feedbackAmount) * cutoff;
		buf1 += (buf0 - buf1) * cutoff;
		switch (mode) {
		case FilterType::LP:
			ochannel.data[i] = buf1; break;
		case FilterType::HP:
			ochannel.data[i] = ichannel.data[i] - buf0;
		case FilterType::BP:
			ochannel.data[i] = buf0 - buf1;
		}
	}
}

void AudioFilter::Load(JSONType& data)
{
	cutoff = (float)data.obj["cutoff"].f;
	resonance = (float)data.obj["resonance"].f;
	mode = (FilterType)data.obj["mode"].i;
}

JSONType AudioFilter::Save()
{
	return JSONType({
		{ "cutoff", (double)cutoff },
		{ "resonance", (double)resonance },
		{ "mode", (long)mode }
	});
}

void DelayNode::Init()
{
	name = "DelayNode";
	title = "Delay";
	minSpace = v2(64.0f, 40.0f);
}

void DelayNode::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("feedback", &feedback, 0.0f, 1.0f, true, true);
	FloatInput("mix", &mix, 0.0f, 1.0f, true, true);
	FloatInput("delay", &time, 0.1f, 1.0f, true, false);
}

void DelayNode::Render(const v2& topLeft, DrawList* dl)
{
	EnsureQueueSize();
	for (int i = 0; i < 127; i++)
	{
		dl->Line(topLeft + v2((float)i / 128.0f * 64.0f, 10.0f + queue[i * skipLength].x * 10.0f), topLeft + v2((float)(i + 1) / 128.0f * 64.0f, 10.0f + queue[i * skipLength + skipLength].x * 10.0f), ImColor(1.0f, 0.0f, 0.0f, 0.5f));
		dl->Line(topLeft + v2((float)i / 128.0f * 64.0f, 10.0f + queue[i * skipLength].y * 10.0f), topLeft + v2((float)(i + 1) / 128.0f * 64.0f, 10.0f + queue[i * skipLength + skipLength].y * 10.0f), ImColor(0.0f, 1.0f, 0.0f, 0.5f));
	}
}

bool DelayNode::OnClick(const v2& clickPosition)
{
	int tcp = (int)(clickPosition.x / 20.0f);
	if (clickPosition.x >= 40.0f || clickPosition.y < 20.0f)
		return false;
	delayType = (DelayType)tcp;
	return true;
}

void DelayNode::Work()
{
	EnsureQueueSize();
	for (int i = 0; i < ichannel.bufferSize; i++)
	{
		queue[queuePointer] = queue[queuePointer] * feedback + ichannel.data[i];
		ochannel.data[i].x = queue[queuePointer].x * mix + ichannel.data[i].x * (1.0f - mix);
		ochannel.data[i].y = queue[(queuePointer + queueSize / 2) % queueSize].y * mix + ichannel.data[i].x * (1.0f - mix);
		queuePointer++;
		queuePointer %= queueSize;
	}
}

void DelayNode::Load(JSONType& data)
{
	feedback = (float)data.obj["feedback"].f;
	mix = (float)data.obj["mix"].f;
	time = (float)data.obj["time"].f;
}

JSONType DelayNode::Save()
{
	return JSONType({
		{ "feedback", (double)feedback },
		{ "mix", (double)mix },
		{ "time", (double)time }
	});
}

void DelayNode::EnsureQueueSize()
{
	if (time < 0.1f)
		time = 0.1f;
	queueSize = (int)(time * ichannel.sampleRate * (delayType == DelayType::PingPong ? 2.0f : 1.0f));
	skipLength = queueSize / 256;
	size_t s = queue.size();

	if (queuePointer >= queueSize)
		queuePointer = 0;

	if (queueSize > s)
	{
		for (int i = 0; i < queueSize - s; i++)
			queue.push_back(v2());
	}
	else if (queueSize < s)
	{
		for (int i = 0; i < s - queueSize; i++)
			queue.pop_back();
	}
}

void ApproxLFO::Init()
{
	name = "ApproxLFO";
	title = "Approx LFO";
	minSpace = v2(100.0f, 50.0f);
}

void ApproxLFO::IO()
{
	FloatInput("frequency", &frequency, 0.05f, 5.0f, true, false);
	FloatOutput("LFO", &output);
}

void ApproxLFO::Render(const v2& topLeft, DrawList* dl)
{
	for (int i = 0; i < 49; i++)
	{
		dl->Line(
			topLeft + v2(i * 2.0f, 25.0f - 25.0f * sinf((float)i / 25.0f * PI)),
			topLeft + v2(i * 2.0f + 2.0f, 25.0f - 25.0f * sinf((float)(i + 1) / 25.0f * PI)),
			ImColor(0.0f, 1.0f, 1.0f)
		);
	}
	float phase = GetPhase() * 100.0f;
	dl->Line(topLeft + v2(phase, 0.0f), topLeft + v2(phase, 50.0f), ImColor(1.0f, 1.0f, 0.0f));
}

bool ApproxLFO::OnClick(const v2& clickPosition)
{
	return false;
}

void ApproxLFO::Work()
{
	output = sinf(2.0f * PI * GetPhase()) * 0.5f + 0.5f;
}

float ApproxLFO::GetPhase() const
{
	float length = 1.0f / frequency;
	return fmodf(AudioChannel::t, length) / length;
}
