#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

#include "App/Nodes/NodeRegistry.h"
#include <filesystem>

void App::Initialize()
{
    RegisterNodes();

    drawStyle.InitColours();

    c.LoadState("networks/init.nn", this);
    c.GenerateAllTextLODs();
    c.InitCanvas();
    RegisterJSONCommands();

    astream.Init();

    n->audioStream = &astream;
}

void App::Update()
{
    while (!astream.QueueFull() && GetAudio()) {}
}

void App::UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime)
{
    bool beginExport = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Console", NULL, Console::instance->enabled))
                Console::instance->enabled = !Console::instance->enabled;
            if (ImGui::MenuItem("Debug", NULL, showDebug))
                showDebug = !showDebug;
            if (ImGui::MenuItem("Export", NULL, nullptr))
                beginExport = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();

    DebugWindow(io, lastFrameTime, averageFrameTime);

    c.CreateWindow(&drawStyle, this);

    static char buf[64] = { 0 };
    if (beginExport)
    {
        ImGui::OpenPopup("Export");
        memset(buf, 0, 64);

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Export", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("name", buf, 64);
        ImGui::DragFloatRange2("time range", &exportBegin, &exportEnd, 0.1f, 0.0f, 10000.0f, "%.2f");

        // disable if filename is empty
        ImGui::BeginDisabled(buf[0] == '\0');
        if (ImGui::Button("Export"))
        {
            // check file does not exist, if so, complain
            std::string filepath = "output/" + std::string(buf) + ".wav";
            if (std::filesystem::exists(filepath))
                ImGui::OpenPopup("Overwriting File");

            else
            {
                // save file
                Export(filepath);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndDisabled();

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

        bool closeBoth = false;
        if (ImGui::BeginPopupModal("Overwriting File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("There already exists a file by this name,\nare you sure you want to overwrite it?");
            if (ImGui::Button("Yes!"))
            {
                Export("output/" + std::string(buf) + ".wav");
                ImGui::CloseCurrentPopup();
                closeBoth = true;
            }
            if (ImGui::Button("no..."))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (closeBoth)
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
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

void App::Export(const std::string& filepath)
{
    std::vector<v2> dataBuf = std::vector<v2>((size_t)((exportEnd - exportBegin) * SAMPLE_RATE) + 1);
    Arranger::instance->setTime(exportBegin);

    astream.doNotMakeSound = true;
    size_t tick = 0;
    while (Arranger::instance->getTime() <= exportEnd)
    {
        GetAudio();
        std::vector<v2> v = astream.GetData();
        for (size_t i = 0; i < v.size(); i++)
            dataBuf[i + tick * BUFFER_SIZE] = v[i];
    }
    astream.doNotMakeSound = false;
    
    std::ofstream wf(filepath, std::ios::out | std::ios::binary);

    if (!wf) {
        Console::LogErr("failed to open file: " + filepath);
        return;
    }

    uint32_t fileSize = 10;
    uint32_t channels = 2;
    uint32_t sampleRate = SAMPLE_RATE;
    uint32_t bitsPerSample = 16;
    uint32_t byteRate = SAMPLE_RATE * channels * bitsPerSample / 8;
    uint32_t blockAlign = channels * bitsPerSample / 8;
    uint32_t dataSize = dataBuf.size() * channels * bitsPerSample / 8;

    wf.write("RIFF", 4);                                                    // "RIFF"           4b be
    wf.write(static_cast<char*>(static_cast<void*>(&fileSize)), 4);         // filesize         4b le
    wf.write("WAVE", 4);                                                    // "WAVE"           4b be
    
    wf.write("fmt ", 4);                                                    // "fmt "           4b be
    wf.write("\x0F\x00\x00\x00", 4);                                        // 16               4b le
    wf.write("\x01\x00", 2);                                                // 1                2b le
    wf.write(static_cast<char*>(static_cast<void*>(&channels)), 2);         // numChannels      2b le
    wf.write(static_cast<char*>(static_cast<void*>(&sampleRate)), 4);       // sampleRate       4b le
    wf.write(static_cast<char*>(static_cast<void*>(&byteRate)), 4);         // byteRate         4b le
    wf.write(static_cast<char*>(static_cast<void*>(&blockAlign)), 2);       // blockAlign       2b le
    wf.write(static_cast<char*>(static_cast<void*>(&bitsPerSample)), 2);    // bitsPerSample    2b le

    wf.write("data", 4); // "data"           4b be
    wf.write(static_cast<char*>(static_cast<void*>(&dataSize)), 4);         // dataSize (bytes) 4b le
    
    // data             *  le
    
    // wf.write((char*)&wstu[i], sizeof(Student));

    wf.close();
}

void App::Release()
{
    astream.Release();

    if (n != nullptr)
        delete n;
}

bool App::GetAudio()
{
    // execute networks, send sound data off
    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f, (float)BUFFER_SIZE / (float)SAMPLE_RATE);
    if (n == nullptr)
    {
        Console::LogWarn("NETWORK EXECUTING SKIPPED");
        return false;
    }
    t_fake += (float)BUFFER_SIZE / (float)SAMPLE_RATE;
    if (!n->Execute()) {
        Console::LogWarn("NETWORK EXECUTING FAILED");
        return false;
    }
    return true;
}
