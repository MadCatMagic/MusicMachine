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
	float getTime(float period) const;

	inline float getTempo() const { return tempo; }
	inline bool paused() const { return !playing; }

private:

	const float pixelsPerBeat = 8.0f;

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

	int scalingLevel = 15;
	float position = 0.0f;
	float scale = 1.0f;

	v2 canvasPixelPos;
	v2 canvasPixelSize;

	DrawList drawList;
};