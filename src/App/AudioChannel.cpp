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

AudioChannel::AudioChannel(const AudioChannel&)
{
	livingChannels.push_back(this);
	ResetData();
}

AudioChannel& AudioChannel::operator=(const AudioChannel&)
{
	livingChannels.push_back(this);
	ResetData();
	return *this;
}


AudioChannel::~AudioChannel()
{
	livingChannels.erase(std::find(livingChannels.begin(), livingChannels.end(), this));
}

void AudioChannel::ResetData()
{
	data = std::vector<v2>(bufferSize, v2());
}




size_t FreqSpaceChannel::sampleRate = 0;
size_t FreqSpaceChannel::bufferSize = 0;

float FreqSpaceChannel::t = 0;
float FreqSpaceChannel::dt = 0;

std::vector<FreqSpaceChannel*> FreqSpaceChannel::livingChannels = std::vector<FreqSpaceChannel*>();

void FreqSpaceChannel::Init(int sr, int bs, float _t, float _dt)
{
	sampleRate = sr;
	bufferSize = bs;
	t = _t;
	dt = _dt;

	for (FreqSpaceChannel* channel : livingChannels)
		channel->ResetData();
}

FreqSpaceChannel::FreqSpaceChannel()
{
	livingChannels.push_back(this);
	ResetData();
}

FreqSpaceChannel::FreqSpaceChannel(const FreqSpaceChannel&)
{
	livingChannels.push_back(this);
	ResetData();
}

FreqSpaceChannel& FreqSpaceChannel::operator=(const FreqSpaceChannel&)
{
	livingChannels.push_back(this);
	ResetData();
	return *this;
}


FreqSpaceChannel::~FreqSpaceChannel()
{
	livingChannels.erase(std::find(livingChannels.begin(), livingChannels.end(), this));
}

void FreqSpaceChannel::ResetData()
{
	left = std::vector<Complex>(bufferSize, Complex());
	right = std::vector<Complex>(bufferSize, Complex());
}
