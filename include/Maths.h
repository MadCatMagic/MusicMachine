#pragma once
#include <vector>
#include <string>

#define PI 3.1415927f
#define TWOPI 6.2831855f

#define RAD_TO_DEG (180.0f / PI)

template <typename T>
inline T clamp(const T& a, const T& min, const T& max)
{
	return a <= min ? min : (a >= max ? max : a);
}

struct Complex {
	inline Complex() : re(0.0f), im(0.0f) { }
	inline Complex(float _re, float _im) : re(_re), im(_im) { }

	float re;
	float im;

	inline Complex operator+(const Complex& a) const { return Complex(re + a.re, im + a.im); }
	inline Complex operator-() const { return Complex(-re, -im); }
	inline Complex operator-(const Complex& a) const { return Complex(re - a.re, im - a.im); }
	inline Complex operator*(float a) const { return Complex(re * a, im * a); }
	inline Complex operator/(float a) const { return Complex(re / a, im / a); }

	Complex operator*(const Complex& a) const;
	Complex operator/(const Complex& a) const;

	Complex& operator+=(const Complex& a);
	Complex& operator-=(const Complex& a);
	Complex& operator*=(float a);
	Complex& operator*=(const Complex& a);
	Complex& operator/=(float a);
	Complex& operator/=(const Complex& a);

	inline bool operator==(const Complex& a) const { return re == a.re && im == a.im; }
	inline bool operator!=(const Complex& a) const { return re != a.re || im != a.im; }

	// returns e^z
	float phase() const;
	float modulus() const;

	Complex exp() const;

	std::string str() const;
};

extern std::vector<Complex> FFT(const std::vector<Complex>& x);
extern std::vector<Complex> IFFT(const std::vector<Complex>& X);
extern std::vector<Complex> _FFT(const std::vector<Complex>& x, size_t n, size_t s = 1, size_t o = 0);
extern std::vector<Complex> _IFFT(const std::vector<Complex>& X, size_t n, size_t s = 1, size_t o = 0);