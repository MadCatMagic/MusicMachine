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

	// either 1 or 2
	int channels = 2;
	static size_t sampleRate;
	static size_t bufferSize;
	static float t;
	static float dt;

	// wasteful, just treat single value for mono channel
	std::vector<v2> data = std::vector<v2>();

private:
	static std::vector<AudioChannel*> livingChannels;
};

struct FreqSpaceChannel
{
	static void Init(int sr, int bs, float _t, float _dt);

	FreqSpaceChannel();
	// copy constructor
	FreqSpaceChannel(const FreqSpaceChannel&);
	FreqSpaceChannel& operator=(const FreqSpaceChannel&);
	~FreqSpaceChannel();

	void ResetData();

	// either 1 or 2
	int channels = 2;
	static size_t sampleRate;
	static size_t bufferSize;
	static float t;
	static float dt;

	std::vector<Complex> left = std::vector<Complex>();
	std::vector<Complex> right = std::vector<Complex>();

private:
	static std::vector<FreqSpaceChannel*> livingChannels;
};