#pragma once
#include "Vector.h"
#include "Engine/DrawList.h"

#define NUM_SCALING_LEVELS 32
#define MIN_SCALE 0.23939204f

class Arranger
{
public:
	static Arranger* instance;
	Arranger();

	void Work();

	void UI(DrawStyle* drawStyle);

	// scaled for tempo
	int getBeat(int mod, float division) const;
	// returns [0,1] for this period (period in terms of beats)
	float getTiming(float period) const;

	inline float getTime() const { return time; }
	inline void setTime(float t) const { time = t; }
	inline float getTempo() const { return tempo; }
	inline bool paused() const { return !playing; }

private:

	const float pixelsPerBeat = 12.0f;
	const float rowHeight = 80.0f;
	const float padding = 6.0f;

	v2 ScreenToCanvas(const v2& pos) const;
	v2 CanvasToScreen(const v2& pos) const;
	v2 CanvasToPosition(const v2& pos) const;
	v2 PositionToCanvas(const v2& pos) const;

	float GetSFFromScalingLevel(int scaling);

	inline v2 ptcts(const v2& pos) const { return CanvasToScreen(PositionToCanvas(pos)); }
	inline v2 stctp(const v2& pos) const { return CanvasToPosition(ScreenToCanvas(pos)); }
	
	// this is beat dependent - if the tempo changes this value does not as it just refers to the beat
	float time = 0.0f;
	float tempo = 120.0f;
	bool playing = false;
	bool draggingTimeCursor = false;

	// variableID, nodeID
	std::vector<std::pair<int, int>> selectedStack;
	bool selectingStuff = false;
	v2 selectionStart;
	bool isDraggingNode = false;

	bool inSelectedStack(int i, int j) const;
	bool isNearLine(const v2& pos, int hovered, int& target) const;

	void DeleteNodesOnSelectedStack();

	v2i scalingLevel = 15;
	v2 position = 0.0f;
	v2 scale = 1.0f;

	v2 canvasPixelPos;
	v2 canvasPixelSize;

	DrawList drawList;
};