#pragma once
#include "App/Nodes/Node.h"

struct Sampler : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:


	AudioChannel c{ };

	static std::vector<std::vector<v2>> samplerData;
	static void PopulateData();

	PitchSequencer seq;
};