#include "App/Nodes/NodeTypes/Panner.h"

void Panner::Init()
{
	name = "Panner";
	title = "Panner";
	minSpace = 0.0f;
}

void Panner::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("pan", &pan, -1.0f, 1.0f, true, true);
}

void Panner::Work(int id)
{
	for (size_t i = 0; i < AudioChannel::bufferSize; i++)
	{
		// if pan = 1 should be (0, xy)
		// if pan = -1 should be (xy, 0)
		// if pan = 0 should be (x, y)
		// if pan = 0.5 should be (0.5x, y + 0.5x
		ochannel.data[i] = v2(
			std::max(0.0f, -pan) * ichannel.data[i].y + (1.0f + std::min(0.0f, -pan)) * ichannel.data[i].x,
			std::max(0.0f, pan) * ichannel.data[i].x + (1.0f + std::min(0.0f, pan)) * ichannel.data[i].y
		);
	}
}

void Panner::Load(JSONType& data)
{
	pan = data.f;
}

JSONType Panner::Save()
{
	return JSONType(pan);
}
