#ifndef L_COMPLEX_H
#define L_COMPLEX_H

#include <math.h>

template<typename T>
struct l_complex{
    T x;
    T y;
};

/**
 * @brief ComplexScale  Multiplication by a constant
 * @param a             complex number
 * @param s             constant
 * @return              complex number
 */
template<typename T>
l_complex<T> ComplexScale(l_complex<T> a, double s)
{
    l_complex<T> c;
    c.x = s * a.x;
    c.y = s * a.y;
    return c;
}

/**
 * @brief ComplexMul    Complex multiplication
 * @param a             complex number
 * @param b             complex number
 * @return              complex number
 */
template<typename T>
l_complex<T> ComplexMul(l_complex<T> a, l_complex<T> b)
{
    l_complex<T> c;
    c.x = a.x * b.x - a.y * b.y;
    c.y = a.x * b.y + a.y * b.x;
    return c;
}

/**
 * @brief ComplexMulConj    Complex conjugate multiplication
 * @param a                 complex number
 * @param b                 complex number
 * @return                  complex number
 */
template<typename T>
l_complex<T> ComplexMulConj(l_complex<T> a, l_complex<T> b) {
    l_complex<T> c;
    c.x = a.x * b.x + a.y * b.y;
    c.y = (-a.x * b.y) + a.y * b.x;
    return c;
}

/**
 * @brief ComplexSum    Sum of complex numbers
 * @param a             complex number
 * @param b             complex number
 * @return              complex number
 */
template<typename T>
l_complex<T> ComplexSum(l_complex<T> a, l_complex<T> b)
{
    l_complex<T> c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
}

/**
 * @brief Expj  Phase to complex number
 * @param phase phase in float
 * @return      complex number
 */
template<typename T>
l_complex<T> Expj(float phase)
{
    l_complex<T> c;
    c.x = cos(phase);
    c.y = sin(phase);
    return c;
}

#endif // L_COMPLEX_H
