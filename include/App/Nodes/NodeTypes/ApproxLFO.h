#pragma once
#include "App/Nodes/Node.h"

// sine, square, downsaw, triangle, upsaw
// 32 samples + 1 extra repeated sample at the end to reduce code overhead
const float lfoWaveData[5][33] = {
	{ 0.0f, 0.1950f, 0.3826f, 0.5555f, 0.7071f, 0.8314f, 0.9238f, 0.9807f, 1.0f, 0.9807f, 0.9238f, 0.8314f, 0.7071f, 0.5555f, 0.3826f, 0.1950f, 0.0f, -0.1950f, -0.3826f, -0.5555f, -0.7071f, -0.8314f, -0.9238f, -0.9807f, -1.0f, -0.9807f, -0.9238f, -0.8314f, -0.7071f, -0.5555f, -0.3826f, -0.1950f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f },
	{ 1.0f, 0.9375f, 0.875f, 0.8125f, 0.75f, 0.6875f, 0.625f, 0.5625f, 0.5f, 0.4375f, 0.375f, 0.3125f, 0.25f, 0.1875f, 0.125f, 0.0625f, 0.0f, -0.0625f, -0.125f, -0.1875f, -0.25f, -0.3125f, -0.375f, -0.4375f, -0.5f, -0.5625f, -0.625f, -0.6875f, -0.75f, -0.8125f, -0.875f, -0.9375f, -1.0f },
	{ 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f, 0.875f, 0.75f, 0.625f, 0.5f, 0.375f, 0.25f, 0.125f, 0.0f, -0.125f, -0.25f, -0.375f, -0.5f, -0.625f, -0.75f, -0.875f, -1.0f, -0.875f, -0.75f, -0.625f, -0.5f, -0.375f, -0.25f, -0.125f, 0.0f },
	{ -1.0f, -0.9375f, -0.875f, -0.8125f, -0.75f, -0.6875f, -0.625f, -0.5625f, -0.5f, -0.4375f, -0.375f, -0.3125f, -0.25f, -0.1875f, -0.125f, -0.0625f, 0.0f, 0.0625f, 0.125f, 0.1875f, 0.25f, 0.3125f, 0.375f, 0.4375f, 0.5f, 0.5625f, 0.625f, 0.6875f, 0.75f, 0.8125f, 0.875f, 0.9375f, 1.0f }
};

struct ApproxLFO : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:

	float GetPhase() const;

	float output = 0.0f;
	float frequency = 1.0f;

	// from 0 to 4
	// Sine, Square, DownSaw, Triangle, UpSaw
	float shape = 0.0f;
};