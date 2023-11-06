#pragma once
#include "Vector.h"

// underlying node structure
// contains all of the actual info about the node
// but has no rendering information, only the layout of how the information should be expressed
// inside UI it should call functions similar to ImGui to be able to display that info

struct Node
{
	
	virtual inline void UI() { }

private:
	v2 position = v2::zero;
	v2 size = v2::one;
};