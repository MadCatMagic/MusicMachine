#pragma once
#include "App/Nodes/Node.h"

struct AnalysisNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work() override;

private:
	const float fftFeedback = 0.25f;
	const float minDB = 70.0f;

	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	std::vector<v2> inputBuffer;
	std::vector<float> fftBuffer;
};