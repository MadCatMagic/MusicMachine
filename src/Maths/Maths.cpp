#include "Maths.h"

std::vector<Complex> FFT(const std::vector<Complex>& x)
{
    auto data = _FFT(x, x.size());
    for (auto& v : data)
        v /= (float)x.size();
    return data;
}

std::vector<Complex> IFFT(const std::vector<Complex>& X)
{
    return _IFFT(X, X.size());
}

std::vector<Complex> _FFT(const std::vector<Complex>& x, size_t n, size_t s, size_t o)
{
    if (n == 1)
        return { x[o] };

    auto even = _FFT(x, n / 2, s * 2, o);
    auto odd = _FFT(x, n / 2, s * 2, o + s);

    std::vector<Complex> X = std::vector<Complex>(n);
    for (size_t k = 0; k < n / 2; k++)
    {
        Complex p = even[k];
        Complex q = odd[k] * (Complex(0.0f, -1.0f) * TWOPI * (float)k / (float)n).exp();
        X[k] = p + q;
        X[k + n / 2] = p - q;
    }

    return X;
}

std::vector<Complex> _IFFT(const std::vector<Complex>& X, size_t n, size_t s, size_t o)
{
    if (n == 1)
        return { X[o] };

    auto even = _IFFT(X, n / 2, s * 2, o);
    auto odd = _IFFT(X, n / 2, s * 2, o + s);

    std::vector<Complex> x = std::vector<Complex>(n);
    for (size_t k = 0; k < n / 2; k++)
    {
        Complex p = even[k];
        Complex q = odd[k] * (Complex(0.0f, 1.0f) * TWOPI * (float)k / (float)n).exp();
        x[k] = p + q;
        x[k + n / 2] = p - q;
    }

    return x;
}

Complex Complex::operator*(const Complex& a) const
{
    return Complex(re * a.re - im * a.im, re * a.im + im * a.re);
}

Complex Complex::operator/(const Complex& a) const
{
    float de = a.re * a.re + a.im * a.im;
    return Complex((re * a.re + im * a.im) / de, (im * a.re - re * a.im) / de);
}

Complex& Complex::operator+=(const Complex& a)
{
    re += a.re;
    im += a.im;
    return *this;
}

Complex& Complex::operator-=(const Complex& a)
{
    re -= a.re;
    im -= a.im;
    return *this;
}

Complex& Complex::operator*=(float a)
{
    re *= a;
    im *= a;
    return *this;
}

Complex& Complex::operator*=(const Complex& a)
{
    re = re * a.re - im * a.im;
    im = re * a.im + im * a.re;
    return *this;
}

Complex& Complex::operator/=(float a)
{
    re /= a;
    im /= a;
    return *this;
}

Complex& Complex::operator/=(const Complex& a)
{
    float de = a.re * a.re + a.im * a.im;
    re = (re * a.re + im * a.im) / de;
    im = (im * a.re - re * a.im) / de;
    return *this;
}

float Complex::phase() const
{
    // thank you computer man for removing my headache
    return atan2f(im, re);
}

float Complex::modulus() const
{
    return sqrtf(re * re + im * im);
}

Complex Complex::exp() const
{
    return Complex(cosf(im), sinf(im)) * expf(re);
}

std::string Complex::str() const
{
    return std::to_string(re) + " + " + std::to_string(im) + "i";
}
