#pragma once
#include "App/Nodes/Node.h"

struct PitchShifter : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	float pitchShift = 0.0f;
	int octaveShift = 0;
	PitchSequencer i{ };
	PitchSequencer o{ };
};