#include "App/AudioChannel.h"

size_t AudioChannel::sampleRate = 0;
size_t AudioChannel::bufferSize = 0;

float AudioChannel::t = 0;
float AudioChannel::dt = 0;

std::vector<AudioChannel*> AudioChannel::livingChannels = std::vector<AudioChannel*>();

void AudioChannel::Init(int sr, int bs, float _t, float _dt)
{
	sampleRate = sr;
	bufferSize = bs;
	t = _t;
	dt = _dt;

	for (AudioChannel* channel : livingChannels)
		channel->ResetData();
}

AudioChannel::AudioChannel()
{
	livingChannels.push_back(this);
	ResetData();
}


AudioChannel::~AudioChannel()
{
	livingChannels.erase(std::find(livingChannels.begin(), livingChannels.end(), this));
}

void AudioChannel::ResetData()
{
	data = std::vector<v2>(bufferSize, v2());
}
