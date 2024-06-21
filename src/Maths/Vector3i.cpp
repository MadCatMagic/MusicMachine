#include "Vector.h"

v3i& v3i::operator+=(const v3i& a)
{
    x += a.x;
    y += a.y;
    z += a.z;
    return *this;
}

v3i& v3i::operator-=(const v3i& a)
{
    x -= a.x;
    y -= a.y;
    z -= a.z;
    return *this;
}

v3i& v3i::operator*=(int a)
{
    x *= a;
    y *= a;
    z *= a;
    return *this;
}

v3i& v3i::operator/=(int a)
{
    x /= a;
    y /= a;
    z /= a;
    return *this;
}

std::string v3i::str() const
{
    return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
}

v3i v3i::zero = v3i();
v3i v3i::one = v3i(1, 1, 1);