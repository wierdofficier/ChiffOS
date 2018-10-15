#ifndef IRQ_H
#define IRQ_H
#include <types.h>
extern void irq_install();
extern void register_device(int irq, u32 (*handler)(u32 r));
extern u32 irq_handler(u32 esp);
#endif
