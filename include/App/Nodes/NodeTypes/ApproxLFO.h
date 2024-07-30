#pragma once
#include "App/Nodes/Node.h"

struct ApproxLFO : public Node
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
	enum LFOShape { Sine, Square, UpSaw, DownSaw, Triangle } shape = LFOShape::Sine;

	float GetPhase() const;

	float output = 0.0f;
	float frequency = 1.0f;

};