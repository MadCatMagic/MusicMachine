#pragma once
#include "Vector.h"
#include "Engine/DrawList.h"
#include "Node.h"

// will act as a file which contains all of the nodes, but only as a reference grid point
// all of the transformations between local and canvas coordinates happen here
// as well as all of the maths for scaling and moving around and such
// but all node interactions happen elsewhere

// CreateWindow renders everything, including all the nodes that are passed to it.
#define NUM_SCALING_LEVELS 32
#define MIN_SCALE 0.23939204f

class Canvas
{
public:
	inline Canvas() {}
	/// IMPLEMENT COPY AND MOVE ASSIGNMENT OPERATORS ???
	~Canvas();

	void InitCanvas();
	bool CreateWindow(DrawStyle* drawStyle, class App* appPointer, int canvasI);

	void SaveLoadWindows(bool beginSaveAs, bool beginLoad, bool beginNodeNetworkLoad, App* appPointer);
	void SaveState(const std::string& filepath);
	void LoadState(const std::string& filepath, App* appPointer);

	v2 ScreenToCanvas(const v2& pos) const;
	v2 CanvasToScreen(const v2& pos) const;
	v2 CanvasToPosition(const v2& pos) const;
	v2 PositionToCanvas(const v2& pos) const;

	inline v2 GetSF() const { return scale; }
	static float GetSFFromScalingLevel(int scaling);

	// shortcut
	inline v2 ptcts(const v2& pos) const { return CanvasToScreen(PositionToCanvas(pos)); }
	inline v2 stctp(const v2& pos) const { return CanvasToPosition(ScreenToCanvas(pos)); }

	static void GenerateAllTextLODs();

	class NodeNetwork* nodes = nullptr;
	class NodeNetworkRenderer* nodeRenderer = nullptr;

	// context menu
	v2 contextMenuClickPos;
	
private:
	bool shouldStayOpen = true;

	inline float optionalClamp(float f, float min, float max, bool doMin, bool doMax) 
	{ 
		float dm = doMin && (f < min) ? min : f;
		return doMax && dm > max ? max : dm;
	}
	// text stuff
	static struct ImFont* textLODs[NUM_SCALING_LEVELS];

	int scalingLevel = 15;
	v2 position = v2::zero;
	v2 scale = v2::one;

	v2 canvasPixelPos;
	v2 canvasPixelSize;

	DrawList drawList;

	// stuff
	// always the top element is the selected item
	std::vector<Node*> selectedStack = std::vector<Node*>();
	bool selectingArea = false;
	v2 selectionStart = v2::zero;

	// connections
	bool draggingConnection = false;
	bool connectionReversed = false;
	std::string connectionOriginName = "";
	Node* connectionOrigin = nullptr;

	// dragging stuff
	bool draggingNodes = false;
	float draggingDistance = 0.0f;

	// sliders
	bool draggingSlider = false;
	bool sliderIsInt = false;
	NodeClickResponse::sliderValueType sliderValue{};
	float sliderDelta = 0.0f;
	float totalSliderMovement = 0.0f;
	float originalSliderValue = 0.0f;

	float sliderMin = 0.0f;
	float sliderMax = 1.0f;
	bool sliderLockMin = false;
	bool sliderLockMax = false;
};