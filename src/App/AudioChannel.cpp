#include "App/AudioChannel.h"

int AudioChannel::sampleRate = 0;
int AudioChannel::bufferSize = 0;

std::vector<AudioChannel*> AudioChannel::livingChannels = std::vector<AudioChannel*>();

void AudioChannel::Init(int sr, int bs)
{
	sampleRate = sr;
	bufferSize = bs;

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
