#include "App/Nodes/NodeTypes/AudioFilter.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

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