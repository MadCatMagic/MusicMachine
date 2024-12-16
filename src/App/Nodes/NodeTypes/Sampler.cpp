#include "App/Nodes/NodeTypes/Sampler.h"
#include "Engine/WAV.h"
#include "Engine/DrawList.h"

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
    FloatInput("fade in", &fadein, 0.0001f, 1.0f, true, true);
    FloatInput("fade out", &fadeout, 0.0001f, 1.0f, true, true);
    AudioOutput("samplerrr", &c);
}

void Sampler::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
    for (int i = 0; i < samplerData.size(); i++)
    {
        if (selectedSample == i)
            dl->RectFilled(topLeft + v2(i * 15.0f, 0.0f), topLeft + v2(i * 15.0f + 15.0f, 20.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
        else
            dl->RectFilled(topLeft + v2(i * 15.0f, 0.0f), topLeft + v2(i * 15.0f + 15.0f, 20.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
    }
}

bool Sampler::OnClick(const NodeClickInfo& info)
{
    if (info.isRight || info.interactionType != 0)
        return false;

    int tcp = (int)(info.pos.x / 15.0f);
    selectedSample = tcp;
    return true;
}

void Sampler::Work(int id)
{
    const float root = 261.63f;

    if (seq.length.size() == 0)
        return;

    size_t sample = 0;
    float increment = 1.0f / (float)AudioChannel::sampleRate;

    for (size_t i = 0; i < seq.length.size(); i++)
    {
        // dont need to bother writing anything as should already be empty
        if (seq.cumulativeSamples[i] < (float)kv[id] * (float)AudioChannel::sampleRate && seq.pitch[i] != 0.0f)
        {
            kv[id] = 0.0f;
            ts[id] = seq.pitch[i] / root;
            vs[id] = seq.velocity[i];
        }

        for (size_t csample = sample; sample < csample + seq.length[i]; sample++)
        {
            c.data[sample] = interp(kv[id], ts[id]) * vs[id];
            kv[id] += increment;
        }
    }
}

void Sampler::Load(JSONType& data)
{
    selectedSample = (int)data.obj["sample"].i;
    fadein = (float)data.obj["fadein"].f;
    fadeout = (float)data.obj["fadeout"].f;
}

JSONType Sampler::Save()
{
    return JSONType({
        { "sample", (long)selectedSample },
        { "fadein", (double)fadein },
        { "fadeout", (double)fadeout }
    });
}

v2 Sampler::interp(float time, float timescale) const
{
    float sample = timescale * time * (float)AudioChannel::sampleRate;// / (float)samplerData[selectedSample].size();
	float lerp = sample - floorf(sample);
	size_t i = (size_t)(sample - lerp + 1e-6f);
    float hat = (float)sample / (float)samplerData[selectedSample].size();
    hat = std::min(1.0f, std::min(hat / fadein, (1.0f - hat) / fadeout));
    if (i + 1 >= samplerData[selectedSample].size())
        return 0.0f;
    return (samplerData[selectedSample][i] * (1.0f - lerp) + samplerData[selectedSample][i + 1] * lerp) * hat;
}

void Sampler::PopulateData()
{
    const int numFiles = 5;
    const std::string files[numFiles] = {
        "res/samples/kick.wav",
        "res/samples/snare.wav",
        "res/samples/closed_hihat.wav",
        "res/samples/open_hihat.wav",
        "res/samples/crash.wav"
    };

    for (int i = 0; i < numFiles; i++)
    {
        WAV f = ResampleWAV(LoadWAVFile(files[i]), AudioChannel::sampleRate);
        samplerData.push_back(f.data);
    }
}
