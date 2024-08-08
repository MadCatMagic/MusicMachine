#pragma once
#include "Vector.h"


class Arranger
{
public:
	static Arranger* instance;
	Arranger();

	void Work();

	void UI();

	// scaled for tempo
	int getBeat(int mod, float division) const;
	// returns [0,1] for this period (period in terms of beats)
	float getTime(float period) const;

	inline float getTempo() const { return tempo; }

private:
	
	// this is beat dependent - if the tempo changes this value does not as it just refers to the beat
	float time = 0.0f;
	float tempo = 120.0f;
};