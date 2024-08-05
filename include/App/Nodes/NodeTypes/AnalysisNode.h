#pragma once
#include "App/Nodes/Node.h"

struct AnalysisNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work() override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	AudioChannel ichannel{ };
	AudioChannel ochannel{ };

	std::vector<Complex> resultLeft;
	std::vector<Complex> resultRight;

	bool splitChannels = false;
};