#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "Engine/JSON.h"
#include "Engine/WAV.h"

#include "App/Nodes/NodeRegistry.h"
#include <filesystem>

App* App::instance = nullptr;

void App::Initialize()
{
    instance = this;

    RegisterNodes();

    drawStyle.InitColours();

    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f, (float)BUFFER_SIZE / (float)SAMPLE_RATE);

    Canvas::GenerateAllTextLODs();
    c.push_back(new Canvas());
    if (std::filesystem::exists("networks/init.nn"))
    {
        c[0]->LoadState("networks/init.nn", this, true);
    }
    else
    {
        NodeNetwork* nodes = new NodeNetwork();
        nodes->AddNodeFromName("AudioOutputNode", v2(100.0f, 0.0f));
        nodes->SaveNetworkToFile("networks/init.nn");
        ReplaceMainNetwork(nodes);
        c[0]->nodes = nodes;
    }
    c[0]->InitCanvas();
    RegisterJSONCommands();

    astream.Init();

    n[0]->audioStream = &astream;
    n[0]->isRoot = true;
}

void App::Update()
{
    for (NodeNetwork* network : n)
        network->Update();
    c[0]->nodes->isRoot = true;
    while (!astream.audioData.full() && GetAudio()) {}
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
                // if ctrl held and such, delete network too
                std::string message = GetNetworkName(n[i]) + (shown ? " - shown" : "");
                bool validForDeletion = false;
                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && n[i]->usedInNetworkNode.none() && !n[i]->isRoot)
                {
                    message = "DELETE " + message;
                    validForDeletion = true;
                }

                if (ImGui::MenuItem(message.c_str()) && !n[i]->isRoot)
                {
                    if (shown)
                    {
                        delete c[ownedCanvas];
                        c.erase(c.begin() + ownedCanvas);
                    }
                    else if (!validForDeletion)
                    {
                        c.push_back(new Canvas());
                        size_t p = c.size() - 1;
                        c[p]->nodes = n[i];
                        c[p]->InitCanvas();
                    }
                    if (validForDeletion)
                    {
                        NodeNetwork* deleting = n[i];
                        DeleteNetwork(deleting);
                        delete deleting;
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
            ImGui::ColorEdit4(drawStyle.colours[i].name.c_str(), &drawStyle.colours[i].col.x, ImGuiColorEditFlags_NoInputs);
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
    astream.audioData.clear();
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

    WAV wavfile;
    wavfile.filepath = filepath;
    wavfile.data = dataBuf;
    wavfile.sampleRate = SAMPLE_RATE;
    SaveWAVFile(wavfile);
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
    if (!n[0]->Execute(true, 0)) {
        Console::LogWarn("NETWORK EXECUTING FAILED");
        return false;
    }
    return true;
}

void App::AddNetwork(NodeNetwork* nodes)
{
    if (n.size() == 0)
        n.push_back(nullptr);
    n.push_back(nodes);
}

void App::AddCanvas(NodeNetwork* nodes)
{
    c.push_back(new Canvas());
    c[c.size() - 1]->nodes = nodes;
    c[c.size() - 1]->InitCanvas();
}

void App::DeleteNetwork(NodeNetwork* nodes)
{
    for (size_t i = 0; i < n.size(); i++)
        if (n[i] == nodes)
        {
            if (i == 0)
                n[0] = nullptr;
            else
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
        NodeNetwork* n_k = n[0];
        n[0] = nodes;
        if (n_k != nullptr)
            n.push_back(n_k);
    }
    n[0]->audioStream = &astream;
    n[0]->isRoot = true;
}

std::pair<NodeNetwork*, int> App::GetNetwork(const std::string& name)
{
    for (NodeNetwork* network : n)
        if (network != nullptr && network->name == name && !network->isRoot && !network->usedInNetworkNode.all())
        {
            for (int i = 0; i < (int)network->usedInNetworkNode.size(); i++)
                if (!network->usedInNetworkNode.test(i))
                    return { network, i };
        }

    if (name == "new network")
        return { nullptr, 0 };

    NodeNetwork* network = new NodeNetwork("networks/" + name);
    AddNetwork(network);
    return { network, 0 };
}

std::string App::GetNetworkName(const NodeNetwork* reference) const
{
    return
        "(" +
        (reference->isRoot ? "root" : std::to_string(reference->usedInNetworkNode.count())) +
        ") " +
        reference->name;
}
