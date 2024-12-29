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

// basically a v2 with extra steps
// used to simplify the FFT algorithms - 
// a complex number is one with a real part and an imaginary part
// where the imaginary part is multiplied by i, a constant where i^2=-1.
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

	// if we represent z as r(cos a + i sin a) = re^(ia)
	// then phase() returns a
	// and modulus() returns r
	float phase() const;
	float modulus() const;

	// returns e^z
	Complex exp() const;

	std::string str() const;
};

// Cooley-tukey FFT algorithm
extern std::vector<Complex> FFT(const std::vector<Complex>& x);
extern std::vector<Complex> IFFT(const std::vector<Complex>& X);