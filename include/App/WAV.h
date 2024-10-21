#pragma once
#include "Vector.h"

struct WAV
{
	std::vector<v2> data;
	std::string filepath;
	size_t sampleRate = 0;
};

static WAV LoadWAVFile(const std::string& filepath);
static void SaveWAVFile(const WAV& wav);

static WAV ResampleWAV(const WAV& wav, size_t newSampleRate);