#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536

#define NAN (0.0 / 0.0)
#define INFINITY (1e300 * 1e300)

float fabs(float x);

double fabsl(double x);

float powf(float base, float power);

double powl(double base, double power);

float exp(float x);

double expl(double x);

float log(float x);

double logl(double x);

int isnanf(float x);

int isnanl(double x);

int isinff(float x);

int isinfl(double x);

#endif //MATH_H
