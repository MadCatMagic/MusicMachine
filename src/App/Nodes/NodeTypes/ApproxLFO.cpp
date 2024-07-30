#include "App/Nodes/NodeTypes/ApproxLFO.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

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

void ApproxLFO::Load(JSONType& data)
{
	frequency = (float)data.obj["frequency"].f;
	shape = (LFOShape)data.obj["shape"].i;
}

JSONType ApproxLFO::Save()
{
	return { {
		{ "frequency", (double)frequency },
		{ "shape", (long)shape }
	} };
}
