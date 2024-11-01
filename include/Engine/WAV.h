#pragma once
#include "Vector.h"

struct WAV
{
	std::vector<v2> data;
	std::string filepath;
	size_t sampleRate = 0;
};

extern WAV LoadWAVFile(const std::string& filepath);
extern void SaveWAVFile(const WAV& wav);

extern WAV ResampleWAV(const WAV& wav, size_t newSampleRate);