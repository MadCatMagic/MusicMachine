#include "App/Nodes/NodeTypes/AnalysisNode.h"
#include "Engine/DrawList.h"

void AnalysisNode::Init()
{
    name = "AnalysisNode";
    title = "Analysis Node";
    minSpace = v2(200.0f, 100.0f);
}

void AnalysisNode::IO()
{
    AudioInput("inp", &ichannel);
    AudioOutput("out", &ochannel);
}

void AnalysisNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
    if (inputChanged && inputBuffer.size() != 0)
    {
        std::vector<Complex> cbufferLeft = std::vector<Complex>(inputBuffer.size());
        std::vector<Complex> cbufferRight = std::vector<Complex>(inputBuffer.size());
        for (int i = 0; i < inputBuffer.size(); i++)
        {
            float window = 0.5f * (1.0f - cosf(TWOPI * (float)i / (float)inputBuffer.size()));
            cbufferLeft[i] = Complex(inputBuffer[i].x, 0.0f) * window;
            cbufferRight[i] = Complex(inputBuffer[i].y, 0.0f) * window;
        }

        cbufferLeft = FFT(cbufferLeft);
        cbufferRight = FFT(cbufferRight);

        points.clear();
        float logConstant = 200.0f / log2f(ichannel.sampleRate / 40.0f);
        for (int i = 0; i < cbufferLeft.size() / 2; i++)
        {
            v2 ampphaseLeft = v2(cbufferLeft[i].modulus(), cbufferLeft[i].phase());
            v2 ampphaseRight = v2(cbufferRight[i].modulus(), cbufferRight[i].phase());

            fftBuffer[i] = fftBuffer[i] * fftFeedback + ((ampphaseLeft.x + ampphaseRight.x) * 0.5f) * (1.0f - fftFeedback);

            float combinedAmp = fftBuffer[i];
            // fit it logarithmically to correct range
            combinedAmp = clamp(20.0f * log10f(combinedAmp), -minDB, 0.0f) + minDB;
            combinedAmp *= 100.0f / minDB;

            float freq = (float)i / (float)(inputBuffer.size()) * ((float)ichannel.sampleRate);

            points.push_back(v2(
                logConstant * log2f(freq / 20.0f) - 0.1f,
                100.0f - combinedAmp
            ));
        }

        inputChanged = false;
    }

    if (points.size() > 1)
    {
        for (size_t i = 0; i < points.size() - 1; i++)
            dl->Line(points[i] + topLeft, points[i + 1] + topLeft, v4(1.0f, 1.0f, 1.0f));
    }
}

void AnalysisNode::Work(int id)
{
    if (inputBuffer.size() == 0)
    {
        inputBuffer = std::vector<v2>(ichannel.bufferSize * 8);
        fftBuffer = std::vector<float>(ichannel.bufferSize * 4);
    }

    inputChanged = true;

    for (size_t i = 0; i < ichannel.bufferSize * 7; i++)
        inputBuffer[i] = inputBuffer[i + ichannel.bufferSize];

    for (int i = 0; i < ichannel.bufferSize; i++)
    {
        inputBuffer[i + ichannel.bufferSize * 7] = ichannel.data[i];
        ochannel.data[i] = ichannel.data[i];
    }
}