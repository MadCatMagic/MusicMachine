#include "Vector.h"
#include <cmath>
#include "imgui.h"

v2::v2(const v2i& i) : x((float)i.x), y((float)i.y) { }
v2::v2(const ImVec2& v) : x(v.x), y(v.y) { }

ImVec2 v2::ImGui() const
{
	return ImVec2(x, y);
}

bool v2::inBox(const v2& bottomLeft, const v2& topRight) const
{
	return bottomLeft.x <= x && bottomLeft.y <= y
		&& topRight.x >= x && topRight.y >= y;
}

float v2::dot(const v2& a) const
{
	return x * a.x + y * a.y;
}

float v2::cross(const v2& a) const
{
	return x * a.y - y * a.x;
}

float v2::angleTo(const v2& a) const
{
	return abs(atan2f(a.y * x - a.x * y, a.x * x + a.y * y));
}

v2 v2::reflect(const v2& axis) const
{
	return *this - axis * (this->dot(axis)) * 2.0f;
}

v2 v2::reciprocal() const
{
	return v2(1.0f / x, 1.0f / y);
}

v2 v2::scale(const v2& a) const
{
	return v2(x * a.x, y * a.y);
}

v2 v2::normalise() const
{
	const float kEpsilon = 0.000001f;
	float mag = length();
	if (mag > kEpsilon)
		return *this / mag;
	else
		return zero;
}

float v2::length() const
{
	return sqrt(x * x + y * y);
}

float v2::length2() const
{
	return x * x + y * y;
}

float v2::distanceTo(const v2& a) const
{
	return (a - *this).length();
}

v2& v2::operator+=(const v2& a)
{
	x += a.x;
	y += a.y;
	return *this;
}

v2& v2::operator-=(const v2& a)
{
	x -= a.x;
	y -= a.y;
	return *this;
}

v2& v2::operator*=(float a)
{
	x *= a;
	y *= a;
	return *this;
}

v2& v2::operator/=(float a)
{
	x /= a;
	y /= a;
	return *this;
}

std::string v2::str() const
{
	return std::to_string(x) + ", " + std::to_string(y);
}

v2 v2::zero = v2();
v2 v2::one = v2(1.0f, 1.0f);