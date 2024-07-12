#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

#include "App/Nodes/NodeRegistry.h"

void App::Initialize()
{
    RegisterNodes();

    drawStyle.InitColours();

    n = new NodeNetwork();
    Node* b = n->AddNodeFromName("AudioOutputNode");
    b->position = v2(100.0f, 0.0f);
    b->IO();
    Node* m = n->AddNodeFromName("SawWave");
    m->IO();
    m->position = v2(-10.0f, 0.0f);
    if (b->Connect(0, m, 0))
        Console::Log("successfully connected nodes");
    else
        Console::LogErr("failed to connect nodes");
    c.GenerateAllTextLODs();
    c.nodes = n;
    c.InitCanvas();
    RegisterJSONCommands();

    astream.Init();
    astream.app = this;

    n->audioStream = &astream;
}

void App::Update()
{
    
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

	ImGui::End();

    c.CreateWindow(&drawStyle, this);
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
    astream.Release();

    if (n != nullptr)
        delete n;
}

void App::GetAudio()
{
    // execute networks, send sound data off
    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, t_fake, (float)BUFFER_SIZE / (float)SAMPLE_RATE);
    if (n == nullptr || c.isDeletingNodes() || c.unsafe)
    {
        Console::LogWarn("NETWORK EXECUTING SKIPPED");
        return;
    }
    t_fake += (float)BUFFER_SIZE / (float)SAMPLE_RATE;
    if (!n->Execute())
        Console::LogWarn("NETWORK EXECUTING FAILED");
}
