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

	inline bool NoData() const { return dataA.size() == 0 && dataB.size() == 0; }

	void Init();
	void Release();

	std::vector<v2> dataA;
	std::vector<v2> dataB;

	bool dataAFirst = true;

	float previousData[1024]{};
	unsigned int previousDataP = 0;

	PaStream* stream;
};