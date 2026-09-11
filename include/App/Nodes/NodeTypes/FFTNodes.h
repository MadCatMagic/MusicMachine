#pragma once
#include "App/Nodes/Node.h"
#include <deque>

struct FFTNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

private:
	AudioChannel ichannel{ };
	FreqSpaceChannel ochannel{ };

	std::deque<v2> inputFIFO;
    std::vector<v2> overlapBuffer;
    std::vector<Complex> frameLeft;
	std::vector<Complex> frameRight;
};

struct IFFTNode : public Node
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Work(int id) override;

private:
	FreqSpaceChannel ichannel{ };
	AudioChannel ochannel{ };

    std::deque<v2> outputFIFO;
    std::vector<v2> overlapBuffer;
    std::vector<float> overlapNorm;

	std::vector<Complex> inverseLeft;
	std::vector<Complex> inverseRight;
};

struct SpectralFilter : public Node 
{
protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;
	virtual bool OnClick(const NodeClickInfo& info) override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	float cutoff = 0.1f;

	FreqSpaceChannel ichannel{ };
	FreqSpaceChannel ochannel{ };
};