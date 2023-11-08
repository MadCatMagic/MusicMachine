#pragma once
#include "Vector.h"

// will act as a file which contains all of the nodes, but only as a reference grid point
// all of the transformations between local and canvas coordinates happen here
// as well as all of the maths for scaling and moving around and such
// but all node interactions happen elsewhere

// CreateWindow renders everything, including all the nodes that are passed to it.

class Canvas
{
public:
	inline Canvas() {}

	void CreateWindow(class NodeNetwork* nodes);

	v2 ScreenToCanvas(const v2& pos) const;
	v2 CanvasToScreen(const v2& pos) const;
	v2 CanvasToPosition(const v2& pos) const;
	v2 PositionToCanvas(const v2& pos) const;

	// shortcut
	inline v2 ptcts(const v2& pos) const { return CanvasToScreen(PositionToCanvas(pos)); }
	
private:
	int scalingLevel = 7;
	v2 position = v2::zero;
	v2 scale = v2::one;

	v2 canvasPixelPos;
	v2 canvasPixelSize;
};