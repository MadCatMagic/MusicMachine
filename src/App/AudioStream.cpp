
#include "App/AudioStream.h"

#include "Engine/Console.h"

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    /* Cast data passed through stream to our structure. */
    AudioStream* data = (AudioStream*)userData;
    float* out = (float*)outputBuffer;
    unsigned int i;
    (void)inputBuffer; /* Prevent unused variable warning. */

    // check if any data is filled; if none is, something went wrong probably
    if (data->NoData())
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

    std::vector<v2>& correctData = data->dataAFirst ? data->dataA : data->dataB;

    for (i = 0; i < framesPerBuffer; i++)
    {
        out[i * 2] = correctData[i].x;  /* left */
        out[i * 2 + 1] = correctData[i].y;  /* right */
        if (i % 8 == 0)
        {
            data->previousData[data->previousDataP++] = correctData[i].x;
            data->previousDataP %= 1024;
        }
    }

    if (data->dataAFirst)
        data->dataA = std::vector<v2>();
    else
        data->dataB = std::vector<v2>();

    data->dataAFirst = !data->dataAFirst;

    return 0;
}

void AudioStream::SetData(std::vector<v2>& v)
{
    if (dataAFirst)
    {
        if (dataA.size() == 0)
            dataA = v;
        else
            dataB = v;
    }
    else
    {
        if (dataB.size() == 0)
            dataB = v;
        else
            dataA = v;
    }
}

void AudioStream::Init()
{
    // audio 
    PaError err = Pa_Initialize();
    if (err != paNoError) goto error;

    //astream = AudioStream(SAMPLE_RATE);

    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream(&stream,
        0,          /* no input channels */
        2,          /* stereo output */
        paFloat32,  /* 32 bit floating point output */
        SAMPLE_RATE,
        BUFFER_SIZE,        /* frames per buffer, i.e. the number
                           of sample frames that PortAudio will
                           request from the callback. Many apps
                           may want to use
                           paFramesPerBufferUnspecified, which
                           tells PortAudio to pick the best,
                           possibly changing, buffer size.*/
        patestCallback, /* this is your callback function */
        this); /*This is a pointer that will be passed to
                           your callback*/
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
