#pragma once
#include "App/Nodes/Node.h"

struct Panner : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float pan = 0.0f;
};