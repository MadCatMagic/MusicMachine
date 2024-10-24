#pragma once
#include "App/Nodes/Node.h"

// sine, square, downsaw, triangle, upsaw
// 32 samples + 1 extra repeated sample at the end to reduce code overhead
const float lfoWaveData[6][33] = {
	{ 0.0f, 0.1950f, 0.3826f, 0.5555f, 0.7071f, 0.8314f, 0.9238f, 0.9807f, 1.0f, 0.9807f, 0.9238f, 0.8314f, 0.7071f, 0.5555f, 0.3826f, 0.1950f, 0.0f, -0.1950f, -0.3826f, -0.5555f, -0.7071f, -0.8314f, -0.9238f, -0.9807f, -1.0f, -0.9807f, -0.9238f, -0.8314f, -0.7071f, -0.5555f, -0.3826f, -0.1950f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f },
	{ 1.0f, 0.9375f, 0.875f, 0.8125f, 0.75f, 0.6875f, 0.625f, 0.5625f, 0.5f, 0.4375f, 0.375f, 0.3125f, 0.25f, 0.1875f, 0.125f, 0.0625f, 0.0f, -0.0625f, -0.125f, -0.1875f, -0.25f, -0.3125f, -0.375f, -0.4375f, -0.5f, -0.5625f, -0.625f, -0.6875f, -0.75f, -0.8125f, -0.875f, -0.9375f, -1.0f },
	{ 1.0f, 0.6531f, 0.3579f, 0.1079f, -0.1024f, -0.2784f, -0.4246f, -0.5453f, -0.6440f, -0.7243f, -0.7888f, -0.8402f, -0.8808f, -0.9124f, -0.9366f, -0.9550f, -0.9688f, -0.9788f, -0.9860f, -0.9910f, -0.9944f, -0.9967f, -0.9981f, -0.9990f, -0.9995f, -0.9998f, -0.9999f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f },
	{ 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f, 0.875f, 0.75f, 0.625f, 0.5f, 0.375f, 0.25f, 0.125f, 0.0f, -0.125f, -0.25f, -0.375f, -0.5f, -0.625f, -0.75f, -0.875f, -1.0f, -0.875f, -0.75f, -0.625f, -0.5f, -0.375f, -0.25f, -0.125f, 0.0f },
	{ -1.0f, -0.9375f, -0.875f, -0.8125f, -0.75f, -0.6875f, -0.625f, -0.5625f, -0.5f, -0.4375f, -0.375f, -0.3125f, -0.25f, -0.1875f, -0.125f, -0.0625f, 0.0f, 0.0625f, 0.125f, 0.1875f, 0.25f, 0.3125f, 0.375f, 0.4375f, 0.5f, 0.5625f, 0.625f, 0.6875f, 0.75f, 0.8125f, 0.875f, 0.9375f, 1.0f }
};

struct ApproxLFO : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:

	float GetPhase() const;

	float output = 0.0f;
	bool useSync = false;
	int syncInput = 0;
	float frequency = 1.0f;

	// from 0 to 5
	// Sine, Square, DownSaw, Parabola, Triangle, UpSaw
	float shape = 0.0f;
};