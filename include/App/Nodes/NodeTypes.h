#pragma once
#include "App/Nodes/Node.h"

struct MathsNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

	inline virtual void Work() override { result = a + b; resultRounded = (int)result + c; }

private:
	float a = 0.0f;
	float b = 0.0f;
	int c = 0;
	int resultRounded = 0;
	float result = 0.0f;
	bool crazy = true;
};

struct LongNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;
	virtual std::string Result() override;

	inline virtual void Work() override { }

private:
	float in1 = 0.0f;
	float in2 = 0.0f;
	float in3 = 0.0f;
	float in4 = 0.0f;
	float in5 = 0.0f;
	float in6 = 0.0f;
};