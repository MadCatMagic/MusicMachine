#include "BBox.h"
#include <cmath>

bbox2::bbox2()
	: a(), b()
{ }

bbox2::bbox2(const v2& p1, const v2& p2)
{
	a = v2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	b = v2(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}

bool bbox2::overlaps(const bbox2& o) const
{
	v2 dd = b - a + o.b - o.a;
	return
		abs(a.x + b.x - o.a.x - o.b.x) <= dd.x &&
		abs(a.y + b.y - o.a.y - o.b.y) <= dd.y;
}

bool bbox2::contains(const v2& p) const
{
	return p.inBox(a, b);
}

bool bbox2::containsLeniant(const v2& p, float leniancy)
{
	return p.inBox(a - leniancy, b + leniancy);
}

bbox2 bbox2::Min(const bbox2& a, const bbox2& b)
{
	return bbox2(
		v2(std::max(a.a.x, b.a.x), std::max(a.a.y, b.a.y)),
		v2(std::min(a.b.x, b.b.x), std::min(a.b.y, b.b.y))
	);
}

bbox2 bbox2::Max(const bbox2& a, const bbox2& b)
{
	return bbox2(
		v2(std::min(a.a.x, b.a.x), std::min(a.a.y, b.a.y)),
		v2(std::max(a.b.x, b.b.x), std::max(a.b.y, b.b.y))
	);
}
