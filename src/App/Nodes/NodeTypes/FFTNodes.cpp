#include "App/Nodes/NodeTypes/FFTNodes.h"
#include "Engine/DrawList.h"
#include "App/AudioStream.h"

const size_t fftSize = BUFFER_SIZE * 2;

void FFTNode::Init()
{
    name = "FFTNode";
    title = "FFT";
    minSpace = v2(20.0f, 10.0f);

    frameLeft = std::vector<Complex>(fftSize);
    frameRight = std::vector<Complex>(fftSize);
}

void FFTNode::IO()
{
    AudioInput("inp", &ichannel);
    FreqSpaceOutput("out", &ochannel);
}

void FFTNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
}

void FFTNode::Work(int id)
{
    inputFIFO.insert(inputFIFO.end(), ichannel.data.begin(), ichannel.data.end());

    bool didSomething = false;
    while (inputFIFO.size() >= fftSize)
    {
        didSomething = true;
        for (size_t i = 0; i < fftSize; i++)
        {
            const float window = 0.5f * (1.0f - cosf(TWOPI * (float)i / (float)(fftSize - 1.0f)));
            frameLeft[i] = Complex(inputFIFO[i].x * window, 0.0);
            frameRight[i] = Complex(inputFIFO[i].y * window, 0.0);
        }

        ochannel.left = FFT(frameLeft);
        ochannel.right = FFT(frameRight);

        inputFIFO.erase(inputFIFO.begin(), inputFIFO.begin() + BUFFER_SIZE);
    }

    if (!didSomething) {
        ochannel.left = std::vector<Complex>(fftSize);
        ochannel.right = std::vector<Complex>(fftSize);
    }
}

void IFFTNode::Init()
{
    name = "IFFTNode";
    title = "IFFT";
    minSpace = v2(20.0f, 10.0f);

    overlapBuffer = std::vector<v2>(fftSize);
    overlapNorm = std::vector<float>(fftSize, 0.0f);
}

void IFFTNode::IO()
{
    FreqSpaceInput("inp", &ichannel);
    AudioOutput("out", &ochannel);
}

void IFFTNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
}

void IFFTNode::Work(int id)
{
    inverseLeft = IFFT(ichannel.left);
    inverseRight = IFFT(ichannel.right);

    for (size_t i = 0; i < fftSize; i++)
    {
        const float window = 0.5f * (1.0f - cosf(TWOPI * (float)i / (float)(fftSize - 1.0f)));
        overlapBuffer[i] += v2(inverseLeft[i].re, inverseRight[i].re) * window;
        overlapNorm[i] += window * window;
    }

    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        v2 sample = v2();
        if (overlapNorm[i] > 1.0e-9)
            sample = overlapBuffer[i] / overlapNorm[i];

        ochannel.data[i] = sample;
    }

    // Shift the overlap buffer by H samples.
    for (size_t i = 0; i < fftSize - BUFFER_SIZE; i++)
    {
        overlapBuffer[i] = overlapBuffer[i + BUFFER_SIZE];
        overlapNorm[i] = overlapNorm[i + BUFFER_SIZE];
    }

    for (size_t i = fftSize - BUFFER_SIZE; i < fftSize; i++)
    {
        overlapBuffer[i] = v2();
        overlapNorm[i] = 0.0f;
    }
}

void SpectralFilter::Init()
{
	name = "SpectralFilter";
	title = "Spectral Filter";
	minSpace = v2(100.0f, 100.0f);
}

void SpectralFilter::IO()
{
	FreqSpaceInput("inp", &ichannel);
	FreqSpaceOutput("out", &ochannel);
	FloatInput("cutoff", &cutoff, 0.0f, 1.0f, true, false, Node::FloatDisplayType::Db);
}

void SpectralFilter::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	
}

bool SpectralFilter::OnClick(const NodeClickInfo& info)
{
	return false;
}

#include "Engine/Console.h"
void SpectralFilter::Work(int id)
{
    //Console::Log("ochannel.left[5]: " + ochannel.left[5].str() + " ; modulus: " + std::to_string(ochannel.left[5].modulus()));
	for (int i = 0; i < ichannel.bufferSize; i++) {
        float leftMod = 0.0;
        float rightMod = 0.0;
        for (int dd = -4; dd <= 4; dd++) {
            if (i + dd < 0 || i + dd >= ichannel.bufferSize) {
                continue;
            }
            leftMod  += ichannel.left[i + dd].modulus() * 2;
            rightMod += ichannel.right[i + dd].modulus() * 2;
        }
        if (leftMod >= cutoff) {
            ochannel.left[i] = ichannel.left[i];
        } else {
            ochannel.left[i] = Complex();
        }
        if (rightMod >= cutoff) {
            ochannel.right[i] = ichannel.right[i];
        } else {
            ochannel.right[i] = Complex();
        }
    }
}

void SpectralFilter::Load(JSONType& data)
{
	cutoff = (float)data.obj["cutoff"].f;
}

JSONType SpectralFilter::Save()
{
	return JSONType((std::unordered_map<std::string, JSONType>){
		{ "cutoff", (double)cutoff }
	});
}