#pragma once
#include "App/Nodes/Node.h"

struct Distortion : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float pregain = 1.0f;
	float distortion = 0.5f;
	float mix = 1.0f;
};