#pragma once
#include "App/Nodes/Node.h"

// atm just doesnt care about multiple instances
// only allowed to be added in the main instance
// maybe change in future
struct AnalysisNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

private:
	const float fftFeedback = 0.25f;
	const float minDB = 70.0f;
	
	bool inputChanged = false;

	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	std::vector<v2> points;
	std::vector<v2> inputBuffer;
	std::vector<float> fftBuffer;
};