#include "App/Nodes/NodeTypes/Distortion.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

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