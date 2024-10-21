#include "App/WAV.h"
#include "Engine/Console.h"
#include <fstream>

WAV LoadWAVFile(const std::string& filepath)
{
    WAV wav;
    wav.filepath = filepath;
    
    std::ifstream f(filepath, std::ios::in | std::ios::binary);

    if (!f) {
        Console::LogErr("failed to open file: " + wav.filepath);
        return wav;
    }
    
    f.read()

    return wav;
}

void SaveWAVFile(const WAV& wav)
{
    std::vector<int16_t> data = std::vector<int16_t>(wav.data.size() * 2);
    for (size_t i = 0; i < wav.data.size(); i++)
    {
        data[i * 2] = (int16_t)(clamp(wav.data[i].x, -1.0f, 1.0f) * INT16_MAX);
        data[i * 2 + 1] = (int16_t)(clamp(wav.data[i].y, -1.0f, 1.0f) * INT16_MAX);
    }

    std::ofstream wf(wav.filepath, std::ios::out | std::ios::binary);

    if (!wf) {
        Console::LogErr("failed to open file: " + wav.filepath);
        return;
    }

    uint32_t channels = 2;
    uint32_t sampleRate = wav.sampleRate;
    uint32_t bitsPerSample = 16;
    uint32_t byteRate = wav.sampleRate * channels * bitsPerSample / 8;
    uint32_t blockAlign = channels * bitsPerSample / 8;
    uint32_t dataSize = (uint32_t)wav.data.size() * channels * bitsPerSample / 8;
    uint32_t fileSize = 36 + dataSize;

    wf.write("RIFF", 4);                                                    // "RIFF"           4b be
    wf.write(static_cast<char*>(static_cast<void*>(&fileSize)), 4);         // filesize         4b le
    wf.write("WAVE", 4);                                                    // "WAVE"           4b be

    wf.write("fmt ", 4);                                                    // "fmt "           4b be
    wf.write("\x10\x00\x00\x00", 4);                                        // 16               4b le
    wf.write("\x01\x00", 2);                                                // 1                2b le
    wf.write(static_cast<char*>(static_cast<void*>(&channels)), 2);         // numChannels      2b le
    wf.write(static_cast<char*>(static_cast<void*>(&sampleRate)), 4);       // sampleRate       4b le
    wf.write(static_cast<char*>(static_cast<void*>(&byteRate)), 4);         // byteRate         4b le
    wf.write(static_cast<char*>(static_cast<void*>(&blockAlign)), 2);       // blockAlign       2b le
    wf.write(static_cast<char*>(static_cast<void*>(&bitsPerSample)), 2);    // bitsPerSample    2b le

    wf.write("data", 4);                                                    // "data"           4b be
    wf.write(static_cast<char*>(static_cast<void*>(&dataSize)), 4);         // dataSize (bytes) 4b le

    wf.write(static_cast<char*>(static_cast<void*>(&data[0])), data.size() * bitsPerSample / 8);

    wf.close();
}

WAV ResampleWAV(const WAV& wav, size_t newSampleRate)
{
    if (wav.sampleRate == newSampleRate)
        return wav;

    WAV newWav;
    newWav.filepath = wav.filepath + "::" + std::to_string(newSampleRate);
    size_t newSize = (size_t)((double)wav.data.size() * ((double)newSampleRate / (double)wav.sampleRate));
    newWav.data = std::vector<v2>(newSize);
    newWav.sampleRate = newSampleRate;

    for (size_t i = 0; i < newSize; i++)
    {
        double s = (double)i / (double)newSize * (double)wav.data.size();
        double f = fmod(s, 1.0f);
        newWav.data.push_back(wav.data[(size_t)s] * (float)(1.0 - f) + wav.data[((size_t)s + 1) % wav.data.size()] * (float)f);
    }
    return newWav;
}
