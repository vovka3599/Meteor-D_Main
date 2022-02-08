#include "l_complex.h"

template<typename T>
l_complex<T> ComplexScale(l_complex<T> a, double s)
{
    l_complex<T> c;
    c.x = s * a.x;
    c.y = s * a.y;
    return c;
}

template<typename T>
l_complex<T> ComplexMul(l_complex<T> a, l_complex<T> b)
{
    l_complex<T> c;
    c.x = a.x * b.x - a.y * b.y;
    c.y = a.x * b.y + a.y * b.x;
    return c;
}

template<typename T>
l_complex<T> ComplexMulConj(l_complex<T> a, l_complex<T> b) {
    l_complex<T> c;
    c.x = a.x * b.x + a.y * b.y;
    c.y = (-a.x * b.y) + a.y * b.x;
    return c;
}

template<typename T>
l_complex<T> ComplexSum(l_complex<T> a, l_complex<T> b)
{
    l_complex<T> c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
}

template<typename T>
l_complex<T> Expj(float phase)
{
    l_complex<T> c;
    c.x = cos(phase);
    c.y = sin(phase);
    return c;
}
