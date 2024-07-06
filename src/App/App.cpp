#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

#include "App/Nodes/NodeFactory.h"
#include "App/Nodes/NodeTypes.h"


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

void App::Initialize()
{
    GetNodeFactory().Register("Node", "Base Node", NodeBuilder<Node>);
    GetNodeFactory().Register("MathsNode", "Maths Node", NodeBuilder<MathsNode>);
    GetNodeFactory().Register("SawWave", "Saw Wave", NodeBuilder<SawWave>);
    GetNodeFactory().Register("AudioOutputNode", "Audio Output Node", NodeBuilder<AudioOutputNode>);
    GetNodeFactory().Register("AudioAdder", "Audio Adder", NodeBuilder<AudioAdder>);
    GetNodeFactory().Register("SequencerNode", "Sequencer Node", NodeBuilder<SequencerNode>);

    drawStyle.InitColours();

    n = new NodeNetwork();
    Node* b = n->AddNodeFromName("Node");
    b->IO();
    Node* m = n->AddNodeFromName("MathsNode");
    m->IO();
    if (m->Connect(0, b, 1))
        Console::Log("successfully connected nodes");
    else
        Console::LogErr("failed to connect nodes");
    c.GenerateAllTextLODs();
    c.nodes = n;
    c.InitCanvas();
    RegisterJSONCommands();

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
        &astream); /*This is a pointer that will be passed to
                           your callback*/
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    n->audioStream = &astream;

    return;
error:
    Console::LogErr("PaError, errored while sound initialising");
    Console::LogErr(std::string(Pa_GetErrorText(err)));
}

void App::Update()
{
    // execute networks, send sound data off
    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, t_fake, (float)BUFFER_SIZE / (float)SAMPLE_RATE);
    t_fake += (float)BUFFER_SIZE / (float)SAMPLE_RATE;
    if (astream.dataA.size() == 0 || astream.dataB.size() == 0)
        if (!n->Execute())
            Console::LogWarn("NETWORK EXECUTING FAILED");
}

void App::UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Console", NULL, Console::instance->enabled))
                Console::instance->enabled = !Console::instance->enabled;
            if (ImGui::MenuItem("Debug", NULL, showDebug))
                showDebug = !showDebug;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();

    DebugWindow(io, lastFrameTime, averageFrameTime);
	
	ImGui::Begin("App");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);

    ImGui::PlotHistogram("previousSamplesLeft", astream.previousData, 1024, 0, 0, -1.0f, 1.0f, ImVec2(0.0f, 40.0f));

	ImGui::End();

    c.CreateWindow(&drawStyle);
}


void App::DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime)
{
    frameTimeWindow[frameTimeI] = (float)lastFrameTime;
    averageTimeWindow[frameTimeI] = (float)averageFrameTime;
    frameTimeI = (++frameTimeI) % FRAME_TIME_MOVING_WINDOW_SIZE;

    if (!showDebug) return;
    ImGui::Begin("Debug", &showDebug);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::Text("Update average %.3f ms/frame (%.1f potential FPS)", averageFrameTime, 1000.0f / averageFrameTime);
    // draw graph
    ImGui::PlotHistogram("frame times", frameTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));
    ImGui::PlotHistogram("avg frame times", averageTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));

    if (ImGui::BeginMenu("Colours"))
    {
        for (int i = 0; i < NUM_DRAW_COLOURS; i++)
            ImGui::ColorEdit4(drawStyle.colours[i].name.c_str(), &drawStyle.colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
        ImGui::EndMenu();
    }

    ImGui::End();
}

void App::Release()
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

    if (n != nullptr)
        delete n;
}
