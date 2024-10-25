#pragma once
#include "App/Nodes/Node.h"

struct RoundNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Work(int id) override;
	
	virtual void Render(const v2& topLeft, class DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	bool floatInput = true;

	float finput = 0.0f;
	float foutput = 0.0f;
	int iinput = 0;
	int ioutput = 0;
};