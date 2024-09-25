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

    c.push_back(new Canvas());
    c.push_back(new Canvas());
    c[0]->LoadState("networks/init.nn", this);
    c[1]->nodes = new NodeNetwork();
    AddNetwork(c[1]->nodes);
    Canvas::GenerateAllTextLODs();
    c[0]->InitCanvas();
    c[1]->InitCanvas();
    RegisterJSONCommands();

    astream.Init();

    n[0]->audioStream = &astream;
    n[0]->isRoot = true;
}

void App::Update()
{
    c[0]->nodes->isRoot = true;
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
        if (ImGui::BeginMenu("Networks"))
        {
            for (size_t i = 0; i < n.size(); i++)
            {
                bool shown = false;
                size_t ownedCanvas = 0;
                for (; ownedCanvas < c.size(); ownedCanvas++)
                    if (c[ownedCanvas]->nodes == n[i])
                    {
                        shown = true;
                        break;
                    }
                // add canvas with network if not shown, otherwise delete existing canvas
                if (ImGui::MenuItem((n[i]->name + (shown ? " - shown" : "")).c_str()) && !n[i]->isRoot)
                {
                    if (shown)
                    {
                        delete c[ownedCanvas];
                        c.erase(c.begin() + ownedCanvas);
                    }
                    else
                    {
                        c.push_back(new Canvas());
                        size_t p = c.size() - 1;
                        c[p]->nodes = n[i];
                        c[p]->InitCanvas();
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();

    DebugWindow(io, lastFrameTime, averageFrameTime);

    ImGui::Begin("Arranger");
    arranger.UI(&drawStyle);
    ImGui::End();

    int toDestroyI = -1;
    for (int i = 0; i < (int)c.size(); i++)
        if (c[i]->CreateWindow(&drawStyle, this, i))
        {
            if (i == 0)
                continue;
            toDestroyI = i;
        }

    if (toDestroyI != -1)
    {
        delete c[toDestroyI];
        c.erase(c.begin() + toDestroyI);
    }

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
    size_t bufSize = (size_t)((exportEnd - exportBegin) * SAMPLE_RATE) + 1;
    std::vector<v2> dataBuf = std::vector<v2>(bufSize);
    Arranger::instance->setTime(exportBegin);

    astream.doNotMakeSound = true;
    astream.EmptyQueue();
    bool wasPlaying = Arranger::instance->playing;
    Arranger::instance->playing = true;

    size_t tick = 0;
    while (Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f <= exportEnd)
    {
        GetAudio();
        std::vector<v2> v = astream.GetData();
        for (size_t i = 0; i < v.size(); i++)
        {
            if (i + tick * BUFFER_SIZE >= bufSize)
                goto escape;
            dataBuf[i + tick * BUFFER_SIZE] = v[i];
        }
        tick += 1;
    }
escape:
    astream.doNotMakeSound = false;
    Arranger::instance->playing = wasPlaying;

    std::vector<int16_t> data = std::vector<int16_t>(dataBuf.size() * 2);
    for (size_t i = 0; i < dataBuf.size(); i++)
    {
        data[i * 2] = (int16_t)(clamp(dataBuf[i].x, -1.0f, 1.0f) * INT16_MAX);
        data[i * 2 + 1] = (int16_t)(clamp(dataBuf[i].y, -1.0f, 1.0f) * INT16_MAX);
    }
    
    std::ofstream wf(filepath, std::ios::out | std::ios::binary);

    if (!wf) {
        Console::LogErr("failed to open file: " + filepath);
        return;
    }

    uint32_t channels = 2;
    uint32_t sampleRate = SAMPLE_RATE;
    uint32_t bitsPerSample = 16;
    uint32_t byteRate = SAMPLE_RATE * channels * bitsPerSample / 8;
    uint32_t blockAlign = channels * bitsPerSample / 8;
    uint32_t dataSize = dataBuf.size() * channels * bitsPerSample / 8;
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

    wf.write("data", 4); // "data"           4b be
    wf.write(static_cast<char*>(static_cast<void*>(&dataSize)), 4);         // dataSize (bytes) 4b le
    
    wf.write(static_cast<char*>(static_cast<void*>(&data[0])), data.size() * bitsPerSample / 8);

    wf.close();
}

void App::Release()
{
    astream.Release();

    for (auto* p : n)
        delete p;

    for (auto* p : c)
        delete p;
}

bool App::GetAudio()
{
    // execute networks, send sound data off
    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f, (float)BUFFER_SIZE / (float)SAMPLE_RATE);
    if (n.size() == 0)
    {
        Console::LogWarn("NETWORK EXECUTING SKIPPED");
        return false;
    }
    arranger.Work();
    if (!n[0]->Execute()) {
        Console::LogWarn("NETWORK EXECUTING FAILED");
        return false;
    }
    return true;
}

void App::AddNetwork(NodeNetwork* nodes)
{
    n.push_back(nodes);
}

void App::DeleteNetwork(NodeNetwork* nodes)
{
    for (size_t i = 0; i < n.size(); i++)
        if (n[i] == nodes)
        {
            n.erase(n.begin() + i);
            return;
        }
}

void App::ReplaceMainNetwork(NodeNetwork* nodes)
{
    if (n.size() == 0)
        n.push_back(nodes);
    else
    {
        delete n[0];
        n[0] = nodes;
    }
    n[0]->audioStream = &astream;
    n[0]->isRoot = true;
}
