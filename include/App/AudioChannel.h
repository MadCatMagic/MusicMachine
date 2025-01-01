#pragma once
#include "Vector.h"

struct AudioChannel
{
	static void Init(int sr, int bs, float _t, float _dt);

	AudioChannel();
	// copy constructor
	AudioChannel(const AudioChannel&);
	AudioChannel& operator=(const AudioChannel&);
	~AudioChannel();

	void ResetData();

	// always 2 (why is this a variable?)
	int channels = 2;
	static size_t sampleRate;
	static size_t bufferSize;
	static float t;
	static float dt;

	std::vector<v2> data = std::vector<v2>();

private:
	static std::vector<AudioChannel*> livingChannels;
};