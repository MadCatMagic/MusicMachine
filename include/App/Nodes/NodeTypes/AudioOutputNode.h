#pragma once
#include "App/Nodes/Node.h"

struct AudioOutputNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;
	virtual AudioChannel* Result() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	v2 previousData[8192]{};
	unsigned int previousDataP = 0;

	int previousDataDivider = 0;

	float volume = 0.2f;
	AudioChannel c{ };
};