#pragma once
#include "App/Node.h"

struct MathsNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	inline virtual void Work() override { result = a + b; }

private:
	float a = 0.0f;
	float b = 0.0f;
	float result = 0.0f;
};

struct LongNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	inline virtual void Work() override { }

private:
	static int inputNum;
	static int outputNum;
	static void ConsoleCommand(std::vector<std::string> arguments);
};