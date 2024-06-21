#pragma once
#include "Nodes/Canvas.h"
#include "Nodes/NodeNetwork.h"

const int FRAME_TIME_MOVING_WINDOW_SIZE = 120;
const int FRAME_TIME_AVERAGE_LENGTH = 10;

#define SAMPLE_RATE (44100)
#include "portaudio.h"

class App
{
public:

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime);

	void Release();

private:
	PaStream* stream;

	DrawStyle drawStyle;

	void DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime);
	bool showDebug = false;

	float frameTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	float averageTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	int frameTimeI = 0;

	NodeNetwork* n = nullptr;
	Canvas c;
};