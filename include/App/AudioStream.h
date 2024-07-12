#pragma once
#include "Vector.h"
#include <vector>

#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define BUFFER_SIZE (1024)

class AudioStream
{
public:
	void SetData(std::vector<v2>& v);

	inline bool NoData() const { return audioData.size() == 0; }

	void Init();
	void Release();

	std::vector<v2> audioData;

	class App* app = nullptr;

	PaStream* stream;
};