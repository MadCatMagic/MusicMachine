#pragma once
#include "App/Nodes/Node.h"

struct SequencerNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, class DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	int width = 16;
	int height = 12;
	const float cellSize = 10.0f;
	std::vector<std::pair<int, float>> data;
	PitchSequencer seq;

	int octaveShift = 0;

	float GetPitch(int i);

	void EnsureDataSize();

	//AudioChannel c;

	int currentBeatIndex = 0;
	float bpm = 120.0f;
	bool tempoSync = true;
	int tempoSyncV = 0;
};