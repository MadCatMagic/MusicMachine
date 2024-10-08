#pragma once
#include "App/Nodes/Node.h"

struct MathsNode : public Node
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

	float inputA = 0.0f;
	float inputB = 0.0f;
	float output = 0.0f;

	enum Operation { Add, Multiply, Subtract, Divide } op = Operation::Add;
};