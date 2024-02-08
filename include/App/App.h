#pragma once
#include "Nodes/Canvas.h"
#include "Nodes/NodeNetwork.h"

class App
{
public:

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io);

	void Release();

private:
	NodeNetwork* n;
	Canvas c;
};