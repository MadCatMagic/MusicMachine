#include "App/Nodes/NodeTypes/Sampler.h"
#include "App/WAV.h"

std::vector<std::vector<v2>> Sampler::samplerData = {};

void Sampler::Init()
{
    if (samplerData.size() == 0)
        PopulateData();

    title = "Sampler";
    name = "Sampler";
    minSpace = v2(15.0f * samplerData.size(), 20.0f);
}

void Sampler::IO()
{
    SequencerInput("seq", &seq);
    AudioOutput("samplerrr", &c);
}

void Sampler::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
}

void Sampler::Work(int id)
{
}

void Sampler::Load(JSONType& data)
{
}

JSONType Sampler::Save()
{
    return JSONType();
}

void Sampler::PopulateData()
{
    const int numFiles = 5;
    const std::string files[numFiles] = {
        "samples/kick.wav",
        "samples/snare.wav",
        "samples/closed_hihat.wav",
        "samples/open_hihat.wav",
        "samples/crash.wav"
    };

    for (int i = 0; i < numFiles; i++)
    {
        WAV f = ResampleWAV(LoadWAVFile(files[i]), AudioChannel::sampleRate);
        samplerData.push_back(f.data);
    }
}
