#pragma once
#include "App/Nodes/Node.h"

struct DelayNode : public Node
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

	enum DelayType { Mono, PingPong } delayType = DelayType::Mono;

	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float feedback = 0.5f;
	float mix = 0.5f;
	float time = 0.3f;

	int queueSize = 16384 * 2;
	int skipLength = queueSize / 128;

	void EnsureQueueSize();

	std::vector<v2> queue;
	int queuePointer = 0;
};