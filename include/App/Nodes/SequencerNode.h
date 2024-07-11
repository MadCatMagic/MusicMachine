#pragma once
#include "Node.h"

struct SequencerNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, class DrawList* dl) override;
	virtual bool OnClick(const v2& clickPosition) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

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