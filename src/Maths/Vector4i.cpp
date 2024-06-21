#include "Vector.h"

v4i& v4i::operator+=(const v4i& a)
{
    x += a.x;
    y += a.y;
    z += a.z;
    w += a.w;
    return *this;
}

v4i& v4i::operator-=(const v4i& a)
{
    x -= a.x;
    y -= a.y;
    z -= a.z;
    w -= a.w;
    return *this;
}

v4i& v4i::operator*=(int a)
{
    x *= a;
    y *= a;
    z *= a;
    w *= a;
    return *this;
}

v4i& v4i::operator/=(int a)
{
    x /= a;
    y /= a;
    z /= a;
    w /= a;
    return *this;
}

std::string v4i::str() const
{
    return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w);
}

v4i v4i::zero = v4i();
v4i v4i::one = v4i(1, 1, 1, 1);