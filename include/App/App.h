#pragma once
#include "Canvas.h"
#include "NodeNetwork.h"

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