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
	static App* instance;

	friend Canvas;

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime);

	void Release();

	bool GetAudio();

	void AddNetwork(NodeNetwork* nodes);
	void DeleteNetwork(NodeNetwork* nodes);
	void ReplaceMainNetwork(NodeNetwork* nodes);

	std::pair<NodeNetwork*, int> GetNetwork(const std::string& name);

private:
	AudioStream astream;

	DrawStyle drawStyle;

	void DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime);
	bool showDebug = false;

	float frameTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	float averageTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	int frameTimeI = 0;

	float t_fake = 0.0f;

	float exportBegin = 0.0f;
	float exportEnd = 10.0f;

	void Export(const std::string& filepath);

	std::vector<NodeNetwork*> n;
	std::vector<Canvas*> c;
	Arranger arranger;
};