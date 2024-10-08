#pragma once
#include "App/Nodes/Node.h"

struct DelayNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	v2 queueLerp(float index, int id) const;

	enum DelayType { Mono, PingPong } delayType = DelayType::Mono;

	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float feedback = 0.5f;
	float mix = 0.5f;
	float time = 0.3f;
	float stereoWideness = 1.0f;

	int queueSize[MAX_OWNED_NETWORKS]{};

	void EnsureQueueSize(int id);

	std::vector<v2> queue[MAX_OWNED_NETWORKS];
	int queuePointer[MAX_OWNED_NETWORKS]{};
};