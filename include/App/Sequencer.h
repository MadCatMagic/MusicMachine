#pragma once

#include "Vector.h"
#include <vector>

struct PitchSequencer
{
	// pitch, number of samples to hold pitch, volume of pitch
	std::vector<float> pitch;
	std::vector<int> length;
	std::vector<float> velocity;
	std::vector<bool> first;

	void CopyTo(PitchSequencer* target);

};