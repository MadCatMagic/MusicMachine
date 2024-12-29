#pragma once
#include "Vector.h"

struct bbox2
{
	bbox2();
	bbox2(const v2& p1, const v2& p2);

	v2 a; // "bottomLeft"
	v2 b; // "topRight"

	// returns true if 'o' overlaps this bbox2
	bool overlaps(const bbox2& o) const;
	// returns true if 'p' is inside this bbox2
	bool contains(const v2& p) const;
	bool containsLeniant(const v2& p, float leniancy);
};