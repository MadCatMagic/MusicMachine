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

struct SawWave : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

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

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	v2 previousData[256]{};
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

struct Distortion : public Node
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
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float pregain = 1.0f;
	float distortion = 0.5f;
	float mix = 1.0f;
};

struct AudioFilter : public Node
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
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float cutoff = 0.99f;
	float resonance = 0.0f;
	float feedbackAmount = 0.0f;
	inline void CalculateFeedbackAmount() { feedbackAmount = resonance + resonance / (1.0f - cutoff); }
	v2 buf0 = 0.0f;
	v2 buf1 = 0.0f;

	enum FilterType { LP, HP, BP } mode = FilterType::LP;
};

struct DelayNode : public Node
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

	enum DelayType { Mono, PingPong } delayType = DelayType::Mono;

	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	float feedback = 0.5f;
	float mix = 0.5f;
	float time = 0.3f;

	int queueSize = 16384 * 2;
	int skipLength = queueSize / 128;

	void EnsureQueueSize();

	std::vector<v2> queue;
	int queuePointer = 0;
};

struct ApproxLFO : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

	//virtual void Load(JSONType& data) override;
	//virtual JSONType Save() override;

private:

	float GetPhase() const;

	float output = 0.0f;

	float frequency = 1.0f;

};