#pragma once
#include "Vector.h"

#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define BUFFER_SIZE (256)
#define MAX_QUEUE_LENGTH (10)

#include "Engine/CircularQueue.h"

class AudioStream
{
public:
	void AddData(std::vector<v2>& v);
	std::vector<v2> GetData();

	void Init();
	void Release();

	CircularQueue<std::vector<v2>> audioData = CircularQueue<std::vector<v2>>(MAX_QUEUE_LENGTH);
	
	bool doNotMakeSound = false;

	PaStream* stream = nullptr;
};