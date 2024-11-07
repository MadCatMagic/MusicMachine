#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Engine/Console.h"
#include "App/App.h"

class Engine
{
public:
	void Mainloop();

	bool CreateWindow(const v2i& windowSize, const std::string& name);

private:
	double lastFrameTime[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	int lastFrameTimeI = 0;

	v2i winSize;

	GLFWwindow* window = nullptr;

	void Initialize();
	void Update();
	void Release();

	// aa
	ImGuiIO* io = nullptr;

	App app;
	Console console;
};

