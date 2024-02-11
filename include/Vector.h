#pragma once

#include <string>

#define PI 3.1415927f
#define TWOPI 6.2831855f

struct v2;
struct v3;
struct v4;
struct v2i;
struct v3i;
struct v4i;

struct v2
{
	float x;
	float y;

	inline v2() : x(0.0f), y(0.0f) { }
	inline v2(float a) : x(a), y(a) { }
	inline v2(float x, float y) : x(x), y(y) { }
	v2(const v2i& i);
	
	v2(const struct ImVec2& v);
	ImVec2 ImGui() const;

	bool inBox(const v2& bottomLeft, const v2& topRight) const;
	float dot(const v2& a) const;
	float cross(const v2& a) const;

	v2 reciprocal() const;
	v2 scale(const v2& a) const;

	v2 normalise() const;
	float length() const;
	float distanceTo(const v2& a) const;
	
	inline v2 operator+(const v2& a) const { return v2(x + a.x, y + a.y); }
	inline v2 operator-() const { return v2(-x, -y); }
	inline v2 operator-(const v2& a) const { return v2(x - a.x, y - a.y); }
	inline v2 operator*(float a) const { return v2(x * a, y * a); }
	inline v2 operator/(float a) const { return v2(x / a, y / a); }
	v2& operator+=(const v2& a);
	v2& operator-=(const v2& a);
	v2& operator*=(float a);
	v2& operator/=(float a);

	inline bool operator==(const v2& a) const { return x == a.x && y == a.y; }
	inline bool operator!=(const v2& a) const { return x != a.x || y != a.y; }
	
	std::string str() const;

	static v2 zero;
	static v2 one;
};

struct v3
{
	float x;
	float y;
	float z;

	inline v3() : x(0.0f), y(0.0f), z(0.0f) { }
	inline v3(float a) : x(a), y(a), z(a) { }
	inline v3(float x, float y, float z) : x(x), y(y), z(z) { }
	v3(const v4& wis1);
	v3(const v3i& i);

	float dot(const v3& a) const;
	v3 cross(const v3& a) const;

	v3 reciprocal() const;
	v3 scale(const v3& a) const;

	v3 normalise() const;
	float length() const;
	float distanceTo(const v3& a) const;

	inline v3 operator+(const v3& a) const { return v3(x + a.x, y + a.y, z + a.z); }
	inline v3 operator-() const { return v3(-x, -y, -z); }
	inline v3 operator-(const v3& a) const { return v3(x - a.x, y - a.y, z - a.z); }
	inline v3 operator*(float a) const { return v3(x * a, y * a, z * a); }
	inline v3 operator/(float a) const { return v3(x / a, y / a, z / a); }
	v3& operator+=(const v3& a);
	v3& operator-=(const v3& a);
	v3& operator*=(float a);
	v3& operator/=(float a);

	inline bool operator==(const v3& a) const { return x == a.x && y == a.y && z == a.z; }
	inline bool operator!=(const v3& a) const { return x != a.x || y != a.y || z != a.z; }
	
	std::string str() const;

	static v3 zero;
	static v3 one;
};

struct v4
{
	float x;
	float y;
	float z;
	float w;

	inline v4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
	inline v4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) { }
	v4(const v3& wtobe1);
	v4(const v4i& i);

	float dot(const v4& a) const;

	v4 reciprocal() const;
	v4 scale(const v4& a) const;

	v4 normalise() const;
	float length() const;
	float distanceTo(const v4& a) const;

	inline v4 operator+(const v4& a) const { return v4(x + a.x, y + a.y, z + a.z, w + a.w); }
	inline v4 operator-() const { return v4(-x, -y, -z, -w); }
	inline v4 operator-(const v4& a) const { return v4(x - a.x, y - a.y, z - a.z, w - a.w); }
	inline v4 operator*(float a) const { return v4(x * a, y * a, z * a, w * a); }
	inline v4 operator/(float a) const { return v4(x / a, y / a, z / a, w / a); }
	v4& operator+=(const v4& a);
	v4& operator-=(const v4& a);
	v4& operator*=(float a);
	v4& operator/=(float a);

	inline bool operator==(const v4& a) const { return x == a.x && y == a.y && z == a.z && w == a.w; }
	inline bool operator!=(const v4& a) const { return x != a.x || y != a.y || z != a.z && w != a.w; }

	std::string str() const;

	static v4 zero;
	static v4 one;
};

struct v2i
{
	int x;
	int y;

	inline v2i() : x(0), y(0) { }
	inline v2i(int a) : x(a), y(a) { }
	inline v2i(int x, int y) : x(x), y(y) { }
	inline explicit v2i(const v2& f) : x((int)f.x), y((int)f.y) { }

	inline v2i operator+(const v2i& a) const { return v2i(x + a.x, y + a.y); }
	inline v2i operator-() const { return v2i(-x, -y); }
	inline v2i operator-(const v2i& a) const { return v2i(x - a.x, y - a.y); }
	inline v2i operator*(int a) const { return v2i(x * a, y * a); }
	inline v2i operator/(int a) const { return v2i(x / a, y / a); }
	v2i& operator+=(const v2i& a);
	v2i& operator-=(const v2i& a);
	v2i& operator*=(int a);
	v2i& operator/=(int a);

	inline bool operator==(const v2i& a) const { return x == a.x && y == a.y; }
	inline bool operator!=(const v2i& a) const { return x != a.x || y != a.y; }

	std::string str() const;

	static v2i zero;
	static v2i one;
};

struct v3i
{
	int x;
	int y;
	int z;

	inline v3i() : x(0), y(0), z(0) { }
	inline v3i(int a) : x(a), y(a), z(a) { }
	inline v3i(int x, int y, int z) : x(x), y(y), z(z) { }
	inline explicit v3i(const v3& f) : x((int)f.x), y((int)f.y), z((int)f.z) { }

	inline v3i operator+(const v3i& a) const { return v3i(x + a.x, y + a.y, z + a.z); }
	inline v3i operator-() const { return v3i(-x, -y, -z); }
	inline v3i operator-(const v3i& a) const { return v3i(x - a.x, y - a.y, z - a.z); }
	inline v3i operator*(int a) const { return v3i(x * a, y * a, z * a); }
	inline v3i operator/(int a) const { return v3i(x / a, y / a, z / a); }
	v3i& operator+=(const v3i& a);
	v3i& operator-=(const v3i& a);
	v3i& operator*=(int a);
	v3i& operator/=(int a);

	inline bool operator==(const v3i& a) const { return x == a.x && y == a.y && z == a.z; }
	inline bool operator!=(const v3i& a) const { return x != a.x || y != a.y || z != a.z; }

	std::string str() const;

	static v3i zero;
	static v3i one;
};

struct v4i
{
	int x;
	int y;
	int z;
	int w;

	inline v4i() : x(0), y(0), z(0), w(0) { }
	inline v4i(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) { }
	inline explicit v4i(const v4& f) : x((int)f.x), y((int)f.y), z((int)f.z), w((int)f.w) { }

	inline v4i operator+(const v4i& a) const { return v4i(x + a.x, y + a.y, z + a.z, w + a.w); }
	inline v4i operator-() const { return v4i(-x, -y, -z, -w); }
	inline v4i operator-(const v4i& a) const { return v4i(x - a.x, y - a.y, z - a.z, w - a.w); }
	inline v4i operator*(int a) const { return v4i(x * a, y * a, z * a, w * a); }
	inline v4i operator/(int a) const { return v4i(x / a, y / a, z / a, w / a); }
	v4i& operator+=(const v4i& a);
	v4i& operator-=(const v4i& a);
	v4i& operator*=(int a);
	v4i& operator/=(int a);

	inline bool operator==(const v4i& a) const { return x == a.x && y == a.y && z == a.z && w == a.w; }
	inline bool operator!=(const v4i& a) const { return x != a.x || y != a.y || z != a.z && w != a.w; }

	std::string str() const;

	static v4i zero;
	static v4i one;
};


// vector hashing operations for use with unordered_map/unordered_set
// https://en.cppreference.com/w/cpp/container/unordered_map/unordered_map
namespace vecHash
{
	template<class vec, class vecType>
	struct KeyHash2 {
		inline std::size_t operator()(const vec& k) const
		{
			return std::hash<vecType>()(k.x) ^
				(std::hash<vecType>()(k.y) << 32);
		};
	};
	template<class vec, class vecType>
	struct KeyHash3 {
		inline std::size_t operator()(const vec& k) const
		{
			return std::hash<vecType>()(k.x) ^
				(std::hash<vecType>()(k.y) << 16) ^
				(std::hash<vecType>()(k.z) << 32);
		};
	};
	template<class vec, class vecType>
	struct KeyHash4 {
		inline std::size_t operator()(const vec& k) const
		{
			return std::hash<vecType>()(k.x) ^
				(std::hash<vecType>()(k.y) << 16) ^ 
				(std::hash<vecType>()(k.z) << 32) ^
				(std::hash<vecType>()(k.w) << 48);
		};
	};

	template<class vec>
	struct KeyEqual {
		inline bool operator()(const vec& lhs, const vec& rhs) const
		{
			return lhs == rhs;
		};
	};
}