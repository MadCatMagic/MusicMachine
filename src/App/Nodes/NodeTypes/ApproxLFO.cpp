#include "App/Nodes/NodeTypes/ApproxLFO.h"
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
	FloatInput("shape", &shape, 0.0f, 4.0f, true, true);
	FloatOutput("LFO", &output);
}

void ApproxLFO::Render(const v2& topLeft, DrawList* dl)
{
	for (float i = 0; i < 32; i++)
	{
		dl->Line(
			topLeft + v2(
				i / 33.0f * 100.0f,       
				25.0f - 25.0f * (lfoWaveData[(int)floorf(shape)][(int)i] * (1.0f - fmodf(shape, 1.0f)) + lfoWaveData[(int)floorf(shape + 1.0f)][(int)i] * fmodf(shape, 1.0f))
			),
			topLeft + v2(
				(i + 1) / 33.0f * 100.0f, 
				25.0f - 25.0f * (lfoWaveData[(int)floorf(shape)][(int)i + 1] * (1.0f - fmodf(shape, 1.0f)) + lfoWaveData[(int)floorf(shape + 1.0f)][(int)i + 1] * fmodf(shape, 1.0f))
			),
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
	// bilinear interpolation
	float phase = GetPhase() * 32.0f;

	float val1 = lfoWaveData[(int)floorf(shape)][(int)floorf(phase)] * (1.0f - fmodf(phase, 1.0f)) + lfoWaveData[(int)floorf(shape)][(int)floorf(phase + 1.0f)] * fmodf(phase, 1.0f);
	float val2 = lfoWaveData[(int)floorf(shape + 1.0f)][(int)floorf(phase)] * (1.0f - fmodf(phase, 1.0f)) + lfoWaveData[(int)floorf(shape + 1.0f)][(int)floorf(phase + 1.0f)] * fmodf(phase, 1.0f);

	output = val1 * (1.0f - fmodf(shape, 1.0f)) + val2 * fmodf(shape, 1.0f);
	// make sure is in range 0-1
	output = output * 0.5f + 0.5f;
}

float ApproxLFO::GetPhase() const
{
	float length = 1.0f / frequency;
	return fmodf(AudioChannel::t, length) / length;
}

void ApproxLFO::Load(JSONType& data)
{
	frequency = (float)data.obj["frequency"].f;
	shape = (float)data.obj["shape"].f;
}

JSONType ApproxLFO::Save()
{
	return { {
		{ "frequency", (double)frequency },
		{ "shape", (double)shape }
	} };
}
