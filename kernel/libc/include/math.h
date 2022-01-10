#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536

#define NAN (0.0 / 0.0)
#define INFINITY (1e300 * 1e300)

float fabs(float);

double fabsl(double);

float powf(float, float);

double powl(double, double);

float exp(float);

double expl(double);

float log(float);

double logl(double);

int isnanf(float);

int isnanl(double);

int isinff(float);

int isinfl(double);

#endif //MATH_H
