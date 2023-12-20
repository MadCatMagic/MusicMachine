#pragma once
#include "Vector.h"

struct bbox2
{
	bbox2();
	bbox2(const v2& p1, const v2& p2);

	v2 a; // "bottomLeft"
	v2 b; // "topRight"

	bool overlaps(const bbox2& o) const;
	bool contains(const v2& p) const;
	bool containsLeniant(const v2& p, float leniancy);

	static bbox2 Min(const bbox2& a, const bbox2& b);
	static bbox2 Max(const bbox2& a, const bbox2& b);
};