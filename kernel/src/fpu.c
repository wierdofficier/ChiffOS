#include <types.h>
#include <video.h>
#include <proc.h>
 	#define CMOS_ADDRESS 0x70
	#define CMOS_DATA 0x71 
	u8 readCMOS(u8 off)
 	{
 	u8 tmp = inb(CMOS_ADDRESS);
 	outb(CMOS_ADDRESS, (tmp & 0x80) | (off & 0x7F));
 	return inb(CMOS_DATA);
 	} 


volatile task_t* FPUTask = 0;

static void fpu_setcw(u16 ctrlword)
{
    __asm__ volatile("fldcw %0;"::"m"(ctrlword));
}

void fpu_install()
{
    if (! (readCMOS(0x14) & BIT(1)) )
    {
        puts("Math Coprocessor not available\n");
        return;
    }

    __asm__ volatile ("finit");

    fpu_setcw(0x37F); 
    u32 cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0)); 
    cr0 |= BIT(3); 
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0)); 
}
#define NAN (__builtin_nanf (""))
float sgn(float x)
{
    if (x < 0)
        return -1;
    if (x > 0)
        return 1;
    return 0;
}
double fabs(double x)
{
    double result;
    __asm__ volatile("fabs" : "=t" (result) : "0" (x));
    return result;
}

double sqrt(double x)
{
    if (x <  0.0)
        return NAN;

    double result;
    __asm__ volatile("fsqrt" : "=t" (result) : "0" (x));
    return result;
}
void fpu_test()
{
    double squareroot = sqrt(2.0);
    squareroot = fabs(squareroot);
    squareroot /= sqrt(2.0);
    if (squareroot == 1.00)
    {
      
        puts(" (FPU Test: OK)\n");

    }
    else
    {

       puts(" (Test ERROR)\n");

    }
}

