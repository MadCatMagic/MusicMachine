#pragma once
#include "App/Nodes/Node.h"

struct ExampleNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;


private:
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float finput = 0.0f;
	int iinput = 10;
	bool binput = true;
	float dbinput = 0.5f;

	PitchSequencer pinput;
	PitchSequencer poutput;
};