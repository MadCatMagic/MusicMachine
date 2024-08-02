#pragma once
#include "App/Nodes/Node.h"

struct ADSRNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

private:
	AudioChannel ochannel{ };
	PitchSequencer isequencer{ };

	float adsr(int sample) const;

	float attack = 0.005f;
	float decay = 0.0f;
	float sustain = 1.0f;
	float release = 0.01f;
};