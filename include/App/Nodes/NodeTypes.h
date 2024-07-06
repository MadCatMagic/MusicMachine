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

struct AudioAdder : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

	virtual void Work() override;

private:
	AudioChannel ic1{ };
	AudioChannel ic2{ };
	AudioChannel oc{ };
	float lerp = 0.5f;
};

struct SawWave : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	//virtual void Load(JSONType& data) override;
	//virtual JSONType Save() override;

	virtual void Work() override;

private:
	AudioChannel c{ };
	float kv = 0.0f;

	PitchSequencer seq;
};

struct SequencerNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	//virtual void Load(JSONType& data) override;
	//virtual JSONType Save() override;

	virtual void Work() override;

private:
	int horizWidth = 16;
	int vertWidth = 12;
	const float cellSize = 10.0f;
	std::vector<std::pair<int, float>> data;
	PitchSequencer seq;

	float GetPitch(int i);

	void EnsureDataSize();

	//AudioChannel c;

	int currentI = 0;
	float bpm = 120.0f;
};

struct AudioOutputNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;
	virtual AudioChannel* Result() override;

	virtual void Work() override;

private:
	float volume = 0.2f;
	AudioChannel c{ };
};