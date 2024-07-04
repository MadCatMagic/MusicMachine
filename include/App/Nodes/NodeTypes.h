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
	AudioChannel ac;
};

struct SawWave : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

	virtual void Work() override;

private:
	AudioChannel c{ };
	float kv = 0.0f;
	float freq = 220.0f;
};

struct AudioOutputNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;
	virtual AudioChannel* Result() override;

private:
	AudioChannel c{ };
};