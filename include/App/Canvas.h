#pragma once
#include "Vector.h"

// will act as a file which contains all of the nodes, but only as a reference grid point
// all of the transformations between local and canvas coordinates happen here
// as well as all of the maths for scaling and moving around and such
// but all node interactions happen elsewhere

class Canvas
{
public:
	Canvas(const v2& screensize);

	v2 CanvasToScreen(const v2& pos) const;
	v2 ScreenToCanvas(const v2& pos) const;

	v2 screenSize = v2::one;
	
private:
	v2 position = v2::zero;
	v2 scale = v2::one;
};