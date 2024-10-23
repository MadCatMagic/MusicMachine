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

    // get end of file
    f.seekg(0, f.end);
    size_t length = f.tellg();
    f.seekg(0, f.beg);
    
    std::string dump = std::string(4, '\0');
    uint16_t channels{};
    uint32_t sampleRate{};
    uint16_t bitsPerSample{};
    uint32_t byteRate{};
    uint16_t blockAlign{};
    uint32_t fileSize{};

    f.read(&dump[0], 4);                                                   // "RIFF"           4b be
    if (dump != "RIFF")
    {
        Console::LogErr("not RIFF file, found '" + dump + "' instead");
        return wav;
    }
    f.read(reinterpret_cast<char*>(&fileSize), sizeof fileSize);           // filesize         4b le
    f.read(&dump[0], 4);                                                   // "WAVE"           4b be
    
    while (f.tellg() != length)
    {
        // get chunk data
        f.read(&dump[0], 4);                                                   // "fmt "           4b be
        uint32_t chunkSize{};
        f.read(reinterpret_cast<char*>(&chunkSize), sizeof chunkSize);

        if (dump == "fmt ")
        {
            f.read(&dump[0], 2);                                                   // 1                2b le
            f.read(reinterpret_cast<char*>(&channels), sizeof channels);           // numChannels      2b le
            f.read(reinterpret_cast<char*>(&sampleRate), sizeof sampleRate);       // sampleRate       4b le
            f.read(reinterpret_cast<char*>(&byteRate), sizeof byteRate);           // byteRate         4b le
            f.read(reinterpret_cast<char*>(&blockAlign), sizeof blockAlign);       // blockAlign       2b le
            f.read(reinterpret_cast<char*>(&bitsPerSample), sizeof bitsPerSample); // bitsPerSample    2b le

            if (channels != 1 && channels != 2)
            {
                Console::LogErr("invalid number of channels: " + std::to_string(channels));
                return wav;
            }

            wav.sampleRate = sampleRate;
        }
        else if (dump == "data")
        {
            // read file data into memory
            if (bitsPerSample == 8)
            {
                std::vector<int8_t> data = std::vector<int8_t>(chunkSize, 0);
                f.read(reinterpret_cast<char*>(&data[0]), chunkSize);

                if (channels == 1)
                    for (size_t i = 0; i < chunkSize; i++)
                        wav.data.push_back((float)data[i] / (float)INT8_MAX);
                else
                    for (size_t i = 0; i < chunkSize / 2; i++)
                        wav.data.push_back(v2(
                            (float)data[i * 2] / (float)INT8_MAX,
                            (float)data[i * 2 + 1] / (float)INT8_MAX
                        ));
            }

            else if (bitsPerSample == 16)
            {
                std::vector<int16_t> data = std::vector<int16_t>(chunkSize / 2, 0);
                f.read(reinterpret_cast<char*>(&data[0]), chunkSize);

                if (channels == 1)
                    for (size_t i = 0; i < chunkSize / 2; i++)
                        wav.data.push_back((float)(*static_cast<int16_t*>(&data[i])) / (float)INT16_MAX);
                else
                    for (size_t i = 0; i < chunkSize / 4; i++)
                        wav.data.push_back(v2(
                            (float)(*static_cast<int16_t*>(&data[i * 2])) / (float)INT16_MAX,
                            (float)(*static_cast<int16_t*>(&data[i * 2 + 1])) / (float)INT16_MAX
                        ));
            }

            else
            {
                Console::LogErr("invalid number of bitsPerSample: " + std::to_string(bitsPerSample));
                return wav;
            }
        }
        else
        {
            f.ignore(chunkSize);
        }
    }

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

    uint16_t channels = 2;
    uint32_t sampleRate = wav.sampleRate;
    uint16_t bitsPerSample = 16;
    uint32_t byteRate = wav.sampleRate * channels * bitsPerSample / 8;
    uint16_t blockAlign = channels * bitsPerSample / 8;
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
