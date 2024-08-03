#pragma once
#include "App/Nodes/Node.h"

struct WaveformGenerator : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	// Sine, Triangle, Square, Saw
	// 0 - 3
	float shape = 0.0f;

	AudioChannel c{ };
	float kv = 0.0f;

	float GetValue(float phase, int shape) const;
	float Bilinear(float phase) const;

	static std::vector<std::vector<float>> waveformData;

	PitchSequencer seq;
};