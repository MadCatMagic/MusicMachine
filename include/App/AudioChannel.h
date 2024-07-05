#pragma once
#include "Vector.h"
#include <vector>

struct AudioChannel
{
	static void Init(int sr, int bs, float _t, float _dt);

	AudioChannel();
	~AudioChannel();

	void ResetData();

	// either 1 or 2
	int channels = 2;
	static int sampleRate;
	static int bufferSize;
	static float t;
	static float dt;

	// wasteful, just treat single value for mono channel
	std::vector<v2> data = std::vector<v2>();

private:
	static std::vector<AudioChannel*> livingChannels;
};