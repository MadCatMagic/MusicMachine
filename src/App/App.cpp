#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"

void App::Initialize()
{
    n = new NodeNetwork();
    Node* b = n->AddNodeFromName("Node");
    b->IO();
    Node* m = n->AddNodeFromName("Maths");
    m->IO();
    if (m->Connect(0, b, 1))
        Console::Log("successfully connected nodes");
    else
        Console::LogErr("failed to connect nodes");
    c.GenerateAllTextLODs();
    c.nodes = n;
}

void App::Update()
{
}

void App::UI(struct ImGuiIO* io)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Console", NULL, Console::instance->enabled))
                Console::instance->enabled = !Console::instance->enabled;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();
	
	ImGui::Begin("App");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);


	ImGui::End();

    c.CreateWindow();
}

void App::Release()
{
    if (n != nullptr)
        delete n;
}
