#pragma once
#include "App/Nodes/Node.h"

struct WaveformGenerator : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work() override;

private:
	// Sine, Saw, Triangle, Square
	enum Shape { Sine, Saw, Triangle, Square } shape = Shape::Saw;

	AudioChannel c{ };
	float kv = 0.0f;

	float GetValue(float phase) const;

	PitchSequencer seq;
};