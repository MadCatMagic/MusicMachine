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
	std::vector<v2> GetData();
	inline bool NoData() const { return audioQueueLength == 0; }
	inline bool QueueFull() const { return audioQueueLength == maxQueueLength; }

	void Init();
	void Release();

	const static int maxQueueLength = 4;
	std::vector<v2> audioData[maxQueueLength]{};
	int audioQueueStart = 0;
	int audioQueueLength = 0;

	PaStream* stream;
};