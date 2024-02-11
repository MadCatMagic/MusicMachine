#include <cmath>
#include "Vector.h"

v4::v4(const v3& wtobe1) : x(wtobe1.x), y(wtobe1.y), z(wtobe1.z), w(1.0f) { }
v4::v4(const v4i& i) : x((float)i.x), y((float)i.y), z((float)i.z), w((float)i.w) { }

float v4::dot(const v4& a) const
{
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

v4 v4::reciprocal() const
{
	return v4(1.0f / x, 1.0f / y, 1.0f / z, 1.0f / w);
}

v4 v4::scale(const v4& a) const
{
	return v4(x * a.x, y * a.y, z * a.z, w * a.w);
}

v4 v4::normalise() const
{
	const float kEpsilon = 0.00001f;
	float mag = length();
	if (mag > kEpsilon)
		return *this / mag;
	else
		return zero;
}

float v4::length() const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

float v4::distanceTo(const v4& a) const
{
	return (a - *this).length();
}

v4& v4::operator+=(const v4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;
	return *this;
}

v4& v4::operator-=(const v4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;
	return *this;
}

v4& v4::operator*=(float a)
{
	x *= a;
	y *= a;
	z *= a;
	w *= a;
	return *this;
}

v4& v4::operator/=(float a)
{
	x /= a;
	y /= a;
	z /= a;
	w /= a;
	return *this;
}

std::string v4::str() const
{
	return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w);
}

v4 v4::zero = v4();
v4 v4::one = v4(1.0f, 1.0f, 1.0f, 1.0f);