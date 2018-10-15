#ifndef TIMER_H
#define TIMER_H
extern void timer_install();
extern volatile u32 timerticks;
extern unsigned long timer_subticks;
extern signed long timer_drift;
extern signed long _timer_drift;
//int sleep(u32 ms);
#endif
