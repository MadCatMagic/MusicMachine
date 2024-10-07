#pragma once
#include "App/Nodes/Node.h"

struct AudioFilter : public Node
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
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float cutoff = 0.99f;
	float resonance = 0.0f;
	float feedbackAmount = 0.0f;
	inline void CalculateFeedbackAmount() { feedbackAmount = resonance + resonance / (1.0f - cutoff); }
	v2 buf0[MAX_OWNED_NETWORKS]{};
	v2 buf1[MAX_OWNED_NETWORKS]{};

	enum FilterType { LP, HP, BP } mode = FilterType::LP;
};