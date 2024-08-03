#pragma once
#include "App/Nodes/Node.h"

struct Distortion : public Node
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

	enum Mode { SoftClip, HardClip, Bitcrush, Sinfold } mode = Mode::SoftClip;

	float convert(float v) const;
	inline float clamp(float v, float min, float max) const { return v >= max ? max : (v <= min ? min : v); }

	float pregain = 1.0f;
	float distortion = 0.5f;
	float mix = 1.0f;

	float tanConstant = 1.0f;
};