#include <cmath>
#include "Vector.h"

v3::v3(const v4& wis1) : x(wis1.x), y(wis1.y), z(wis1.z) { }
v3::v3(const v3i& i) : x((float)i.x), y((float)i.y), z((float)i.z) { }

float v3::dot(const v3& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

v3 v3::cross(const v3& a) const
{
	return v3(
		y * a.z - z * a.y,
		z * a.x - x * a.z,
		x * a.y - y * a.x
	);
}

v3 v3::reciprocal() const
{
	return v3(1.0f / x, 1.0f / y, 1.0f / z);
}

v3 v3::scale(const v3& a) const
{
	return v3(x * a.x, y * a.y, z * a.z);
}

v3 v3::normalise() const
{
	const float kEpsilon = 0.00001f;
	float mag = length();
	if (mag > kEpsilon)
		return *this / mag;
	else
		return zero;
}

float v3::length() const
{
	return sqrt(x * x + y * y + z * z);
}

float v3::distanceTo(const v3& a) const
{
	return (a - *this).length();
}

v3& v3::operator+=(const v3& a)
{
	this->x += a.x;
	this->y += a.y;
	this->z += a.z;
	return *this;
}

v3& v3::operator-=(const v3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	return *this;
}

v3& v3::operator*=(float a)
{
	x *= a;
	y *= a;
	z *= a;
	return *this;
}

v3& v3::operator/=(float a)
{
	x /= a;
	y /= a;
	z /= a;
	return *this;
}

std::string v3::str() const
{
	return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
}

v3 v3::zero = v3();
v3 v3::one = v3(1.0f, 1.0f, 1.0f);