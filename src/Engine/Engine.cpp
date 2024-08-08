#include "Engine.h"
// safeguarding against wrong include order
#if !(defined(__gl_h_) || defined(__GL_H__) || defined(_GL_H) || defined(__X_GL_H))
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <iostream>

Engine::Engine()
{
}

void Engine::Mainloop(bool debugging)
{
    this->debug = debugging;

    if (window == nullptr)
    {
        std::cout << "[Fatal Error]: Create a window before calling MainLoop()!\n";
        return;
    }

    Initialize();

    while (!glfwWindowShouldClose(window))
    {
        double frameStartTime = glfwGetTime();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        winSize = v2i(display_w, display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Update();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        lastFrameTime[lastFrameTimeI] = (glfwGetTime() - frameStartTime) * 1000.0f;
        lastFrameTimeI = (++lastFrameTimeI) % FRAME_TIME_MOVING_WINDOW_SIZE;

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    Release();

    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Engine::CreateWindow(const v2i& windowSize, const std::string& name)
{
    this->winSize = windowSize;

    /* Initialize the library */
    if (!glfwInit())
        return false;

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(winSize.x, winSize.y, name.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "GLEW ERROR" << std::endl;
        return false;
    }

    return true;
}

void Engine::Initialize()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // actual stuff
    app.Initialize();
}

void Engine::Update()
{
    // actual stuff first
    app.Update();

    // make sure console does its thing
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent))
        console.enabled = !console.enabled;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // average frame time over last 10 frames
    // kinda finickery
    double ft = 0.0f;
    for (int i = 0; i < FRAME_TIME_AVERAGE_LENGTH; i++)
        ft += lastFrameTime[(lastFrameTimeI - i - 1 + FRAME_TIME_MOVING_WINDOW_SIZE) % FRAME_TIME_MOVING_WINDOW_SIZE];
    
    app.UI(io, ft / FRAME_TIME_AVERAGE_LENGTH, lastFrameTime[(lastFrameTimeI - 1 + FRAME_TIME_MOVING_WINDOW_SIZE) % FRAME_TIME_MOVING_WINDOW_SIZE]);
    console.GUI();

    // Rendering
    ImGui::Render();
}

void Engine::Release()
{
    app.Release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
