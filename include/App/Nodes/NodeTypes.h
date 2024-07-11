#pragma once
#include "App/Nodes/Node.h"

struct AudioTransformer : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

private:
	enum TransformationType { Lerp, Multiply } type = TransformationType::Lerp;
	const int numTypes = 2;

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

struct AudioOutputNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;
	virtual AudioChannel* Result() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;

	virtual void Work() override;

private:
	v2 previousData[1024]{};
	unsigned int previousDataP = 0;

	float volume = 0.2f;
	AudioChannel c{ };
};

struct ADSRNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

private:
	AudioChannel ochannel{ };
	PitchSequencer isequencer{ };

	float adsr(int sample) const;

	float attack = 0.005f;
	float decay = 0.0f;
	float sustain = 1.0f;
	float release = 0.01f;
};