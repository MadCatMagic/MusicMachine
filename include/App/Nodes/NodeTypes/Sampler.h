#pragma once
#include "App/Nodes/Node.h"

struct Sampler : public Node
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

	v2 interp(float time, float timescale) const;

	AudioChannel c{ };

	float fadein = 0.01f;
	float fadeout = 0.05f;
	int selectedSample = 0;
	float kv[MAX_OWNED_NETWORKS]{};
	float ts[MAX_OWNED_NETWORKS]{};
	float vs[MAX_OWNED_NETWORKS]{};

	static std::vector<std::vector<v2>> samplerData;
	static void PopulateData();

	PitchSequencer seq;
};