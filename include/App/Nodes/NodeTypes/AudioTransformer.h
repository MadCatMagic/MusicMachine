#pragma once
#include "App/Nodes/Node.h"

struct AudioTransformer : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	enum TransformationType { Lerp, Multiply } type = TransformationType::Lerp;
	const int numTypes = 2;

	AudioChannel ic1{ };
	AudioChannel ic2{ };
	AudioChannel oc{ };
	float lerp = 0.5f;
};