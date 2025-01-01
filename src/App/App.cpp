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

    // create root canvas
    Canvas::GenerateAllTextLODs();
    c.push_back(new Canvas());

    // check if init.nn exists to load
    if (std::filesystem::exists("networks/init.nn"))
        c[0]->LoadState("networks/init.nn", this, true);
    else
    {
        // if not, create a new network and save to init.nn
        NodeNetwork* nodes = new NodeNetwork();
        nodes->AddNodeFromName("AudioOutputNode", v2(100.0f, 0.0f));
        nodes->SaveNetworkToFile("networks/init.nn");
        ReplaceMainNetwork(nodes);
        c[0]->nodes = nodes;
    }
    c[0]->InitCanvas();

    RegisterJSONCommands();

    // initialises port audio
    astream.Init();

    // make sure current network is root
    n[0]->audioStream = &astream;
    n[0]->isRoot = true;
}

void App::Update()
{
    // update networks - checks for cycles in networks
    for (NodeNetwork* network : n)
        network->Update();
    // if a new network is loaded as root this ensures it knows it is root
    c[0]->nodes->isRoot = true;
    // loop to fill up audioData - GetAudio() adds audio to astream
    // runs until audioData is full
    while (!astream.audioData.full() && GetAudio()) {}
}

void App::UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime)
{
    bool beginExport = false;
    // creates the main menu bar at the top of the screen
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
                // loop to find whether each network is being displayed by a canvas,
                // and if so which canvas it is
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
                // only deletes the network if it is not used anywhere, otherwise just removes the canvas
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

    // draw debug window and arranger
    DebugWindow(io, lastFrameTime, averageFrameTime);
    ImGui::Begin("Arranger");
    arranger.UI(&drawStyle);
    ImGui::End();
    
    // only one CreateWindow call can return true (meaning it should be deleted)
    // each frame, so this checks for that and then deletes the canvas if so
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

    // popups for exporting audio
    static char buf[64] = { 0 };
    if (beginExport)
    {
        ImGui::OpenPopup("Export");
        memset(buf, 0, 64);

        // always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Export", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("name", buf, 64);
        ImGui::DragFloatRange2("time range", &exportBegin, &exportEnd, 0.1f, 0.0f, 10000.0f, "%.2f");

        // disable export button if filename is empty
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

        // overwriting file popup
        // if they decide to overwrite the file it has to close both popups
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
    // adds data to circular buffers
    frameTimeWindow[frameTimeI] = (float)lastFrameTime;
    averageTimeWindow[frameTimeI] = (float)averageFrameTime;
    frameTimeI = (++frameTimeI) % FRAME_TIME_MOVING_WINDOW_SIZE;

    if (!showDebug) return;
    ImGui::Begin("Debug", &showDebug);

    // show framerate and 'potential' frame rate
    // since vsync is on, potential fps approximates what the max fps could be if it was not locked to 60Hz.
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::Text("Update average %.3f ms/frame (%.1f potential FPS)", averageFrameTime, 1000.0f / averageFrameTime);
    // draw graphs of frame time over time
    ImGui::PlotHistogram("frame times", frameTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));
    ImGui::PlotHistogram("avg frame times", averageTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));

    // just shows all the colours for the current drawStyle
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
    // create a buffer of the right length for the data
    size_t bufSize = (size_t)((exportEnd - exportBegin) * SAMPLE_RATE) + 1;
    std::vector<v2> dataBuf = std::vector<v2>(bufSize);
    
    // reset audio streams and arranger to the beginning of the required section
    Arranger::instance->setTime(exportBegin);
    astream.doNotMakeSound = true;
    astream.audioData.clear();
    bool wasPlaying = Arranger::instance->playing;
    Arranger::instance->playing = true;

    // loop through buffer and fill with audio data
    size_t tick = 0;
    while (Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f <= exportEnd)
    {
        GetAudio();
        std::vector<v2> v = astream.GetData();
        for (size_t i = 0; i < v.size(); i++)
        {
            // if we have exceeded buffer size, (buffer size does not match up with end of astream buffer)
            // then escape the loop
            if (i + tick * BUFFER_SIZE >= bufSize)
                goto escape;
            // otherwise just add sample to buffer
            dataBuf[i + tick * BUFFER_SIZE] = v[i];
        }
        tick += 1;
    }

    // reset arranger and astream to settings before exporting audio
escape:
    astream.doNotMakeSound = false;
    Arranger::instance->playing = wasPlaying;

    // exports audio info as a WAV file
    WAV wavfile;
    wavfile.filepath = filepath;
    wavfile.data = dataBuf;
    wavfile.sampleRate = SAMPLE_RATE;
    SaveWAVFile(wavfile);
}

void App::Release()
{
    // clean up stuff
    astream.Release();

    for (auto* p : n)
        delete p;

    for (auto* p : c)
        delete p;
}

bool App::GetAudio()
{
    // check that audio can be outputted first
    AudioChannel::Init(SAMPLE_RATE, BUFFER_SIZE, Arranger::instance->getTime() / Arranger::instance->getTempo() * 60.0f, (float)BUFFER_SIZE / (float)SAMPLE_RATE);
    if (n.size() == 0)
    {
        Console::LogWarn("NETWORK EXECUTING SKIPPED");
        return false;
    }
    // then actually execute root network
    arranger.Work();
    if (!n[0]->Execute(true, 0)) {
        Console::LogWarn("NETWORK EXECUTING FAILED");
        return false;
    }
    return true;
}

void App::AddNetwork(NodeNetwork* nodes)
{
    // this is so a new network does not become root if it is not supposed to be
    if (n.size() == 0)
        n.push_back(nullptr);
    // add network to array
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
                // keep *something* in n[0], even if just a nullptr
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
    // first tries to find a matching existing network with space
    for (NodeNetwork* network : n)
        if (network != nullptr && network->name == name && !network->isRoot && !network->usedInNetworkNode.all())
        {
            for (int i = 0; i < (int)network->usedInNetworkNode.size(); i++)
                if (!network->usedInNetworkNode.test(i))
                    return { network, i };
        }

    // then just skips if the network name is invalid
    if (name == "new network")
        return { nullptr, 0 };

    // then tries to load network
    NodeNetwork* network = new NodeNetwork("networks/" + name);
    AddNetwork(network);

    return { network, 0 };
}

std::string App::GetNetworkName(const NodeNetwork* reference) const
{
    // just pads name with some extra info
    return
        "(" +
        (reference->isRoot ? "root" : std::to_string(reference->usedInNetworkNode.count())) +
        ") " +
        reference->name;
}