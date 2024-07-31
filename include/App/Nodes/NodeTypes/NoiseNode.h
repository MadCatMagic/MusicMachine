#pragma once
#include "App/Nodes/Node.h"

struct NoiseNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Work() override;

private:
	AudioChannel c{ };
};