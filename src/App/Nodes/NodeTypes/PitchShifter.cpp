#include "App/Nodes/NodeTypes/PitchShifter.h"

void PitchShifter::Init()
{
	title = "Pitch Shifter";
	name = "PitchShifter";
	minSpace = 0;
}

void PitchShifter::IO()
{
	SequencerInput("i", &i);
	FloatInput("pitch shift", &pitchShift, -12.0f, 12.0f);
	IntInput("octave shift", &octaveShift, -3, 3);
	SequencerOutput("o", &o);
}

void PitchShifter::Work(int id)
{
	i.CopyTo(&o);
	for (size_t i = 0; i < o.pitch.size(); i++)
	{
		if (o.pitch[i] == 0.0f)
			continue;
		o.pitch[i] *= powf(2.0f, octaveShift) * powf(2.0f, pitchShift / 12.0f);
	}
}
