#pragma once
#include "App/Nodes/Node.h"

struct MixNode : public Node
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
	enum TransformationType { WeightedAdd, Multiply } type = TransformationType::WeightedAdd;
	const int numTypes = 2;

	std::vector<AudioChannel> ichannels;
	AudioChannel ochannel{ };
	std::vector<float> weights;
	int numChannels = 2;

	void EnsureCorrectChannelNum();
};