#pragma once
#include "App/Nodes/Node.h"

struct WaveformGenerator : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Work() override;

private:
	AudioChannel c{ };
	float kv = 0.0f;

	PitchSequencer seq;
};