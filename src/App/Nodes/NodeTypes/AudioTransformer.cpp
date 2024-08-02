#include "App/Nodes/NodeTypes/AudioTransformer.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

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

void AudioTransformer::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	for (int i = 0; i < numTypes; i++)
	{
		if ((int)type == i)
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 20.0f, 0.0f), topLeft + v2(i * 20.0f + 20.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}
}

bool AudioTransformer::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0)
		return false;

	int tcp = (int)(info.pos.x / 20.0f);
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