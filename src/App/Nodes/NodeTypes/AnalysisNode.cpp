#include "App/Nodes/NodeTypes/AnalysisNode.h"
#include "Engine/DrawList.h"

void AnalysisNode::Init()
{
    name = "AnalysisNode";
    title = "Analysis Node";
    minSpace = v2(200.0f, 120.0f);
}

void AnalysisNode::IO()
{
    AudioInput("inp", &ichannel);
    AudioOutput("out", &ochannel);
}

void AnalysisNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
    if (resultLeft.size() != 0 && resultRight.size() != 0)
    { 
        dl->Rect(topLeft, topLeft + v2(200.0f, 100.0f), ImColor(0.1f, 0.2f, 0.3f));
        std::vector<v2> points;
        float logConstant = 200.0f / log2f(ichannel.sampleRate / 40.0f);
        for (int i = 0; i < resultLeft.size() / 2; i++)
        {
            v2 ampphaseLeft = v2(resultLeft[i].modulus(), resultLeft[i].phase());
            v2 ampphaseRight = v2(resultRight[i].modulus(), resultRight[i].phase());

            float combinedAmp = (ampphaseLeft.x + ampphaseRight.x) * 0.5f;
            // fit it logarithmically to correct range
            const float minDB = 80.0f;
            combinedAmp = clamp(20.0f * log10f(combinedAmp), -minDB, 0.0f) + minDB;
            combinedAmp *= 100.0f / minDB;

            float freq = (float)i / (float)(ichannel.bufferSize) * ((float)ichannel.sampleRate);

            
            points.push_back(v2(
                logConstant * log2f(freq / 20.0f),
                100.0f - combinedAmp
            ));
        }

        for (size_t i = 0; i < points.size() - 1; i++)
            dl->Line(points[i] + topLeft, points[i + 1] + topLeft, ImColor(1.0f, 1.0f, 1.0f));
    }

    if (splitChannels)
    {
        dl->RectFilled(topLeft + v2(0.0f, 100.0f), topLeft + v2(100.0f, 120.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
        dl->RectFilled(topLeft + v2(100.0f, 100.0f), topLeft + v2(200.0f, 120.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
    }
    else
    {
        dl->RectFilled(topLeft + v2(0.0f, 100.0f), topLeft + v2(100.0f, 120.0f), ImColor(0.2f, 0.3f, 0.7f, 0.8f));
        dl->RectFilled(topLeft + v2(100.0f, 100.0f), topLeft + v2(200.0f, 120.0f), ImColor(0.1f, 0.2f, 0.5f, 0.3f));
    }
}

bool AnalysisNode::OnClick(const NodeClickInfo& info)
{
    if (info.interactionType != 0 || info.isRight || info.pos.y < 100.0f)
        return false;

    splitChannels = info.pos.x > 100.0f;
    return true;
}

void AnalysisNode::Work()
{
    std::vector<Complex> cbufferLeft = std::vector<Complex>(ichannel.bufferSize);
    std::vector<Complex> cbufferRight = std::vector<Complex>(ichannel.bufferSize);
    for (int i = 0; i < ichannel.bufferSize; i++)
    {
        float window = 0.5f * (1.0f - cosf(TWOPI * (float)i / (float)ichannel.bufferSize));
        cbufferLeft[i] = Complex(ichannel.data[i].x, 0.0f) * window;
        cbufferRight[i] = Complex(ichannel.data[i].y, 0.0f) * window;
        ochannel.data[i] = ichannel.data[i];
    }

    resultLeft = FFT(cbufferLeft);
    resultRight = FFT(cbufferRight);
}

void AnalysisNode::Load(JSONType& data)
{
    splitChannels = data.b;
}

JSONType AnalysisNode::Save()
{
    return splitChannels;
}
