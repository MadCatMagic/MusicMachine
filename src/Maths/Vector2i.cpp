#include "Vector.h"

v2i& v2i::operator+=(const v2i& a)
{
    x += a.x;
    y += a.y;
    return *this;
}

v2i& v2i::operator-=(const v2i& a)
{
    x -= a.x;
    y -= a.y;
    return *this;
}

v2i& v2i::operator*=(int a)
{
    x *= a;
    y *= a;
    return *this;
}

v2i& v2i::operator/=(int a)
{
    x /= a;
    y /= a;
    return *this;
}

std::string v2i::str() const
{
    return std::to_string(x) + ", " + std::to_string(y);
}

v2i v2i::zero = v2i();
v2i v2i::one = v2i(1, 1);