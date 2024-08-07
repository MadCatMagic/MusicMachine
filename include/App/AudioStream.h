#pragma once
#include "Vector.h"

#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define BUFFER_SIZE (512)
#define MAX_QUEUE_LENGTH (6)

class AudioStream
{
public:
	void SetData(std::vector<v2>& v);
	std::vector<v2> GetData();
	inline bool NoData() const { return audioQueueLength == 0; }
	inline bool QueueFull() const { return audioQueueLength == MAX_QUEUE_LENGTH; }

	void Init();
	void Release();

	std::vector<v2> audioData[MAX_QUEUE_LENGTH]{};
	int audioQueueStart = 0;
	int audioQueueLength = 0;

	PaStream* stream;
};