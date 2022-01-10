#include <math.h>

#include <stdlib.h>

float fabs(float x) {
    return fabsl((double)x);
}

double fabsl(double x) {
    return x < 0 ? -x : x;
}

float powf(float base, float power) {
    return (float)powl((double)base, (double)power);
}

double powl(double base, double power) {
    // a^b = exp(b * ln(a))
    return expl(power * logl(base));
}

float exp(float x) {
    return (float)expl((double)x);
}

double expl(double x) {
    // Use Taylor series to calculate e^x
    // e^x = 1 + x + x^2/2! + x^3/3! + ...

    double sum = 1.0;
    double term = 1.0;

    for (int n = 1; fabsl(term) > 0.00001; n++) {
        term *= x / n;
        sum += term;
    }

    return sum;
}

float log(float x) {
    return (float)logl((double)x);
}

double logl(double x) {
    // Use Halley-Newton method to calculate log(x)
    // y[n+1] = y[n] + 2 * (x - exp(y[n])) / (x + exp(y[n]))

    double yn = x - 1.0F;
    double yn1 = yn;

    do {
        yn = yn1;
        yn1 = yn + 2.0F * (x - exp(yn)) / (x + exp(yn));
    } while (fabs(yn1 - yn) > 0.00001F);

    return yn1;
}

int isnanf(float x) {
    // NaN is not equal to itself
    return x != x;
}

int isnanl(double x) {
    // NaN is not equal to itself
    return x != x;
}

int isinff(float x) {
    // exponents are all 1s
    x = fabs(x);
    long int p = *(long int *)&x;
    return (p >> 23) == 0xFF;
}

int isinfl(double x) {
    // exponent are all 1
    x = fabsl(x);
    long long int p = *(long long int *)&x;
    return (p >> 52) == 0x7FF;
}
