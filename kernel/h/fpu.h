#ifndef FPU_H
#define FPU_H
float sgn(float x);

double fabs(double x);
void fpu_install();
void fpu_test();
double sqrt(double x);
u8 readCMOS(u8 off);
#endif
