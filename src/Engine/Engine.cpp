#include "Engine.h"
// safeguarding against wrong include order
#if !(defined(__gl_h_) || defined(__GL_H__) || defined(_GL_H) || defined(__X_GL_H))
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <iostream>

void Engine::Mainloop()
{
    // check window exists before trying to run anything
    if (window == nullptr)
    {
        std::cout << "[Fatal Error]: Create a window before calling MainLoop()!\n";
        return;
    }

    Initialize();

    while (!glfwWindowShouldClose(window))
    {
        double frameStartTime = glfwGetTime();

        // get window dimensions and clear the screen in preparation for rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        winSize = v2i(display_w, display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update any non-rendering code
        Update();
        // draw data recieved from ImGui to the screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // record frame time for use with performance measuring
        lastFrameTime[lastFrameTimeI] = (glfwGetTime() - frameStartTime) * 1000.0f;
        lastFrameTimeI = (++lastFrameTimeI) % FRAME_TIME_MOVING_WINDOW_SIZE;

        // update the screen to show rendered stuff
        glfwSwapBuffers(window);
        // poll for events - user interactions like mouse movements, key presses, etc.
        glfwPollEvents();
    }

    Release();

    // finally destroy the window and exit
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Engine::CreateWindow(const v2i& windowSize, const std::string& name)
{
    this->winSize = windowSize;

    // initialize the library
    if (!glfwInit())
        return false;

    // create a windowed mode window and its OpenGL context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(winSize.x, winSize.y, name.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    // make the window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // initialise glew
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
    // setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // setup ImGui style
    ImGui::StyleColorsDark();

    // setup platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // actually initialise application
    app.Initialize();
}

void Engine::Update()
{
    // first update actual app
    app.Update();

    // make sure console can be toggled with grave key
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent))
        console.enabled = !console.enabled;

    // start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // average frame time over last 10 frames
    double ft = 0.0f;
    for (int i = 0; i < FRAME_TIME_AVERAGE_LENGTH; i++)
        ft += lastFrameTime[(lastFrameTimeI - i - 1 + FRAME_TIME_MOVING_WINDOW_SIZE) % FRAME_TIME_MOVING_WINDOW_SIZE];
    
    // draw the UI of app and console
    app.UI(io, ft / FRAME_TIME_AVERAGE_LENGTH, lastFrameTime[(lastFrameTimeI - 1 + FRAME_TIME_MOVING_WINDOW_SIZE) % FRAME_TIME_MOVING_WINDOW_SIZE]);
    console.GUI();

    // render everything
    ImGui::Render();
}

void Engine::Release()
{
    app.Release();

    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
