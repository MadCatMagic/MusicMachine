#pragma once
#include "Nodes/Canvas.h"
#include "Nodes/NodeNetwork.h"
#include "App/Arranger.h"

const int FRAME_TIME_MOVING_WINDOW_SIZE = 120;
const int FRAME_TIME_AVERAGE_LENGTH = 10;

#include "App/AudioStream.h"

class App
{
public:

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime);

	void Release();

	bool GetAudio();

	inline void SetNodes(NodeNetwork* nodes) { n = nodes; n->audioStream = &astream; }

private:
	AudioStream astream;

	DrawStyle drawStyle;

	void DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime);
	bool showDebug = false;

	float frameTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	float averageTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	int frameTimeI = 0;

	float t_fake = 0.0f;

	NodeNetwork* n = nullptr;
	Canvas c;
};