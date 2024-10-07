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
	FloatInput("cutoff", &cutoff, 0.0f, 0.99f, true, true, Node::FloatDisplayType::Hz);
	FloatInput("resonance", &resonance, 0.01f, 0.99f, true, true);
}

void AudioFilter::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < 3; i++)
	{
		if ((int)mode == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}

	if (lodOn)
		return;

	const std::vector<std::vector<v2>> lineData = {
		{ v2(-0.7f, 0.0f), v2(0.1f, 0.0f), v2(0.7f, 0.6f) },
		{ v2(0.7f, 0.0f), v2(-0.1f, 0.0f), v2(-0.7f, 0.6f) },
		{ v2(-0.7f, 0.6f), v2(-0.1f, 0.0f), v2(0.1f, 0.0f), v2(0.7f, 0.6f) }
	};

	for (int j = 0; j < 3; j++)
	{
		std::vector<v2> k;
		for (const v2& v : lineData[j])
			k.push_back(v * 10.0f + topLeft + v2(10.0f + j * 20.0f, 10.0f));
		dl->Lines(k, ImColor(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);
	}
}

bool AudioFilter::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);
	mode = (FilterType)tcp;
	return true;
}

void AudioFilter::Work(int id)
{
	CalculateFeedbackAmount();

	if (isnan(buf0[id].x) || isnan(buf0[id].y))
	{
		Console::LogWarn("NANS IN THE FILTER MAN THERE ARE NANS EVERYWHERE");
		buf0[id] = 0.0f;
		buf1[id] = 0.0f;
	}

	for (int i = 0; i < ichannel.bufferSize; i++)
	{
		buf0[id] += (ichannel.data[i] - buf0[id] + (buf0[id] - buf1[id]) * feedbackAmount) * cutoff;
		buf1[id] += (buf0[id] - buf1[id]) * cutoff;
		switch (mode) {
		case FilterType::LP:
			ochannel.data[i] = buf1[id]; break;
		case FilterType::HP:
			ochannel.data[i] = ichannel.data[i] - buf0[id]; break;
		case FilterType::BP:
			ochannel.data[i] = buf0[id] - buf1[id]; break;
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