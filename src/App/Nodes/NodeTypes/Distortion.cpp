#include "App/Nodes/NodeTypes/Distortion.h"
#include "Engine/Console.h"
#include "Engine/DrawList.h"

void Distortion::Init()
{
	name = "Distortion";
	title = "Distortion";
	minSpace = v2(100.0f, 100.0f);
}

void Distortion::IO()
{
	AudioInput("inp", &ichannel);
	AudioOutput("out", &ochannel);
	FloatInput("pregain", &pregain, 0.0f, 1.0f, true, false, Node::FloatDisplayType::Db);
	FloatInput("distortion", &distortion, 0.0f, 1.0f, true, true);
	FloatInput("mix", &mix, 0.0f, 1.0f, true, true);
}

void Distortion::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	tanConstant = tanf(1.0f / (2.7f - 2.05f * distortion));

	dl->Line(topLeft + v2(50.0f, 20.0f), topLeft + v2(50.0f, 100.0f), ImColor(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);
	dl->Line(topLeft + v2(0.0f, 60.0f), topLeft + v2(100.0f, 60.0f), ImColor(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);

	int skip = lodOn ? 4 : 1;
	for (float i = 0; i < 127; i += skip)
		dl->Line(
			topLeft + v2(
				(float)i / 128.0f * 100.0f,
				60.0f - 40.0f * (convert((float)i / 64.0f - 1.0f) * mix + ((float)i / 64.0f - 1.0f) * (1.0f - mix))
			),
			topLeft + v2(
				(float)(i + skip) / 128.0f * 100.0f,
				60.0f - 40.0f * (convert((float)(i + skip) / 64.0f - 1.0f) * mix + ((float)(i + skip) / 64.0f - 1.0f) * (1.0f - mix))
			),
			ImColor(0.0f, 1.0f, 1.0f)
		);

	for (int i = 0; i < 4; i++)
	{
		if ((int)mode == i)
			dl->RectFilled(topLeft + v2(i * 25.0f, 0.0f), topLeft + v2(i * 25.0f + 25.0f, 18.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
		else
			dl->RectFilled(topLeft + v2(i * 25.0f, 0.0f), topLeft + v2(i * 25.0f + 25.0f, 18.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
	}

	if (lodOn)
		return;

	// SoftClip, HardClip, Bitcrush, Sinfold
	const std::vector<std::vector<v2>> lineData = {
		{ 
			v2(-0.7f, 0.7f), v2(-0.5687f, 0.6834f), v2(-0.4375f, 0.6573f), v2(-0.35f, 0.6298f), v2(-0.2625f, 0.5859f), 
			v2(-0.175f, 0.5072f), v2(-0.1312f, 0.4411f), v2(-0.0875f, 0.342f), v2(-0.0437f, 0.1943f), v2(0.0f, -0.0f), 
			v2(0.0437f, -0.1943f), v2(0.0875f, -0.342f), v2(0.1312f, -0.4411f), v2(0.175f, -0.5072f), v2(0.2625f, -0.5859f), 
			v2(0.35f, -0.6298f), v2(0.4375f, -0.6573f), v2(0.5687f, -0.6834f), v2(0.7f, -0.7f)
		},
		{ v2(-0.7f, 0.7f), v2(-0.3f, 0.7f), v2(0.3f, -0.7f), v2(0.7f, -0.7f) },
		{ v2(-0.7f, 0.7f), v2(-0.35f, 0.7f), v2(-0.35f, 0.35f), v2(0.0f, 0.35f), v2(0.0f, 0.0f), v2(0.35f, 0.0f), v2(0.35f, -0.35f), v2(0.7f, -0.35f), v2(0.7f, -0.7f) },
		{	
			v2(-0.7f, 0.0f), v2(-0.6708f, 0.0022f), v2(-0.6417f, -0.0145f), v2(-0.6125f, -0.06f),
			v2(-0.5833f, -0.1292f), v2(-0.5542f, -0.1967f), v2(-0.525f, -0.2213f), v2(-0.4958f, -0.137f), v2(-0.4667f, 0.0106f),
			v2(-0.4375f, 0.1971f), v2(-0.4083f, 0.387f), v2(-0.3792f, 0.5441f), v2(-0.35f, 0.6409f), v2(-0.3208f, 0.6656f),
			v2(-0.2917f, 0.6223f), v2(-0.2625f, 0.5283f), v2(-0.2333f, 0.4077f), v2(-0.2042f, 0.2843f), v2(-0.175f, 0.1766f),
			v2(-0.1458f, 0.0953f), v2(-0.1167f, 0.0426f), v2(-0.0875f, 0.0144f), v2(-0.0583f, 0.003f), v2(-0.0292f, 0.0002f),
			v2(0.0f, -0.0f), v2(0.0292f, -0.0002f), v2(0.0583f, -0.003f), v2(0.0875f, -0.0144f), v2(0.1167f, -0.0426f),
			v2(0.1458f, -0.0953f), v2(0.175f, -0.1766f), v2(0.2042f, -0.2843f), v2(0.2333f, -0.4077f), v2(0.2625f, -0.5283f), 
			v2(0.2917f, -0.6223f), v2(0.3208f, -0.6656f), v2(0.35f, -0.6409f), v2(0.3792f, -0.5441f), v2(0.4083f, -0.387f),
			v2(0.4375f, -0.1971f), v2(0.4667f, -0.0106f), v2(0.4958f, 0.137f), v2(0.525f, 0.2213f), v2(0.5542f, 0.1967f),
			v2(0.5833f, 0.1292f), v2(0.6125f, 0.06f), v2(0.6417f, 0.0145f), v2(0.6708f, -0.0022f), v2(0.7f, -0.0f)
		}
	};

	for (int j = 0; j < 4; j++)
	{
		std::vector<v2> k;
		for (const v2& v : lineData[j])
			k.push_back(v.scale(v2(12.5f, 9.0f)) + topLeft + v2(12.5f + j * 25.0f, 9.0f));
		dl->Lines(k, ImColor(1.0f, 1.0f, 1.0f), 1.0f / dl->scaleFactor);
	}
}

bool Distortion::OnClick(const NodeClickInfo& info)
{
	if (info.isRight || info.interactionType != 0 || info.pos.y > 18.0f)
		return false;

	int tcp = (int)(info.pos.x / 25.0f);
	mode = (Mode)tcp;
	return true;
}

void Distortion::Work()
{
	tanConstant = tanf(1.0f / (2.7f - 2.05f * distortion));

	for (size_t i = 0; i < ichannel.bufferSize; i++)
	{
		v2 pm = ichannel.data[i] * pregain;
		v2 v = v2(
			convert(pm.x),
			convert(pm.y)
		);
		
		ochannel.data[i] = v * mix + ichannel.data[i] * (1.0f - mix);
	}
}

void Distortion::Load(JSONType& data)
{
	pregain = (float)data.obj["pregain"].f;
	distortion = (float)data.obj["distortion"].f;
	mix = (float)data.obj["mix"].f;
	mode = (Mode)data.obj["mode"].i;
}

JSONType Distortion::Save()
{
	return JSONType({
		{ "pregain", (double)pregain },
		{ "distortion", (double)distortion },
		{ "mix", (double)mix },
		{ "mode", (long)mode }
	});
}

float Distortion::convert(float v) const
{
	//return v >= 0.0f ? powf(v, 1.0f - distortion) : -powf(-v, 1.0f - distortion);
	switch (mode)
	{
	case Mode::SoftClip: return (2.7f - 2.05f * distortion) * atanf(v * tanConstant);
	case Mode::HardClip: return clamp((distortion + 0.166666666f) * 6.0f * v, -1.0f, 1.0f);

	case Mode::Bitcrush: 
	{
		float k = distortion / 1.5f + 0.001f;
		if (abs(v) < k / 20.0f)
			return 0.0f;
		return clamp(floorf(v / k) * k + k * 0.5f, -1.0f, 1.0f);
	}

	case Mode::Sinfold: 
	{
		float k = powf(0.8f * distortion + 1.0f, 7.0f * distortion);
		float tv = 13.0f * sinf(v * v * (v < 0.0f ? -1.0f : 1.0f) * k) * expf(-5.0f * v * v) * v * v;
		return tv * std::min(1.0f, 4.0f - abs(4.0f * v));
	}
	}
	return v;
}
