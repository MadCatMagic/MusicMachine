#pragma once
#include "Vector.h"

struct bbox2
{
	bbox2();
	bbox2(const v2& p1, const v2& p2);

	v2 a; // "bottomLeft"
	v2 b; // "topRight"

	bool overlaps(const bbox2& o);
	bool contains(const v2& p);

	static bbox2 Min(const bbox2& a, const bbox2& b);
	static bbox2 Max(const bbox2& a, const bbox2& b);
};