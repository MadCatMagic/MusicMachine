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
	minSpace = v2(0.0f, 0.0f);
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

void SpectralFilter::Work(int id)
{
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

void SpectralSmear::Init()
{
	name = "SpectralSmear";
	title = "Spectral Smear";
	minSpace = v2(0.0f, 0.0f);

    feedbackLeft = std::vector<Complex>(fftSize);
    feedbackRight = std::vector<Complex>(fftSize);
}

void SpectralSmear::IO()
{
	FreqSpaceInput("inp", &ichannel);
	FreqSpaceOutput("out", &ochannel);
	FloatInput("feedback", &feedback, 0.0f, 1.0f, true, false);
    IntInput("diffusion width", &diffusionWidth, 0, 8, true, false);
    FloatInput("diffusion amount", &diffusionAmount, 0.0f, 1.0f, true, true);
}

void SpectralSmear::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	
}

bool SpectralSmear::OnClick(const NodeClickInfo& info)
{
	return false;
}

void SpectralSmear::Work(int id)
{
    //Console::Log("ochannel.left[5]: " + ochannel.left[5].str() + " ; modulus: " + std::to_string(ochannel.left[5].modulus()));
	for (int i = 0; i < ichannel.bufferSize; i++) {
        feedbackLeft[i] *= feedback;
        feedbackRight[i] *= feedback;

        feedbackLeft[i] += ichannel.left[i];
        feedbackRight[i] += ichannel.right[i];
        
        ochannel.left[i] = feedbackLeft[i];
        ochannel.right[i] = feedbackRight[i];
        // diffuse
        float accumLeft = 0.0f;
        float accumRight = 0.0f;
        for (int j = -diffusionWidth; j <= diffusionWidth; j++) {
            if (i + j < 0 || i + j >= fftSize) {
                continue;
            }
            accumLeft += ochannel.left[i + j].modulus();
            accumRight += ochannel.right[i + j].modulus();
        }
        const float newModLeft = feedbackLeft[i].modulus() * (1.0f - diffusionAmount) + accumLeft * diffusionAmount / (float)(diffusionWidth * 2 + 1);
        const float newModRight = feedbackRight[i].modulus() * (1.0f - diffusionAmount) + accumRight * diffusionAmount / (float)(diffusionWidth * 2 + 1);
        if (feedbackLeft[i].modulus() >= 1e-8)
            feedbackLeft[i] *= newModLeft / feedbackLeft[i].modulus();
        if (feedbackRight[i].modulus() >= 1e-8)
            feedbackRight[i] *= newModRight / feedbackRight[i].modulus();
    }
}

void SpectralSmear::Load(JSONType& data)
{
	feedback = (float)data.obj["feedback"].f;
    diffusionAmount = (float)data.obj["diffAm"].f;
    diffusionWidth = (int)data.obj["diffWi"].i;
}

JSONType SpectralSmear::Save()
{
	return JSONType((std::unordered_map<std::string, JSONType>){
		{ "feedback", (double)feedback },
        { "diffAm", (double)diffusionAmount },
        { "diffWi", (long)diffusionWidth },
	});
}