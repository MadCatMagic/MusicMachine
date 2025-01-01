#include "App/AudioStream.h"

#include "Engine/Console.h"
#include "App/App.h"

// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
static int paCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    AudioStream* data = (AudioStream*)userData;
    float* out = (float*)outputBuffer;
    unsigned int i;
    (void)inputBuffer;

    // check if any data is filled; if none is, something went wrong probably
    if (data->audioData.empty() || data->doNotMakeSound)
    {
        Console::LogWarn("NO DATA!!!");
        // wipe clean
        for (i = 0; i < framesPerBuffer; i++)
        {
            out[i * 2] = 0.0f;
            out[i * 2 + 1] = 0.0f;
        }
        return 0;
    }

    // set data to output buffer
    auto audioData = data->GetData();
    for (i = 0; i < framesPerBuffer; i++)
    {
        out[i * 2] = audioData[i].x;
        out[i * 2 + 1] = audioData[i].y;
    }

    return 0;
}

void AudioStream::AddData(std::vector<v2>& v)
{
    if (audioData.full())
    {
        Console::LogErr("Tried adding audio data queue while queue is full!");
        return;
    }
    audioData.push(v);
}

std::vector<v2> AudioStream::GetData()
{
    if (audioData.empty())
    {
        Console::LogErr("Tried getting audio data from queue while queue is empty!");
        return {};
    }
    return audioData.pop();
}

void AudioStream::Init()
{
    // audio 
    PaError err = Pa_Initialize();
    if (err != paNoError) goto error;

    // open an audio I/O stream
    err = Pa_OpenDefaultStream(&stream,
        0,              // no input channels
        2,              // stereo output
        paFloat32,      // 32 bit floating point output
        SAMPLE_RATE,
        BUFFER_SIZE,    // samples per buffer
        paCallback, // callback function
        this);          // a pointer that will be passed to the callback by PA
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    return;
error:
    Console::LogErr("PaError, errored while sound initialising");
    Console::LogErr(std::string(Pa_GetErrorText(err)));
}

void AudioStream::Release()
{
    PaError err = Pa_StopStream(stream);
    if (err != paNoError)
        Console::Log("PortAudio error: " + std::string(Pa_GetErrorText(err)));

    err = Pa_CloseStream(stream);
    if (err != paNoError)
        Console::Log("PortAudio error: " + std::string(Pa_GetErrorText(err)));

    err = Pa_Terminate();
    if (err != paNoError)
        Console::Log("PortAudio error: " + std::string(Pa_GetErrorText(err)));
}