#ifndef IDT_H
#define IDT_H
#include <types.h>
struct idt_entry
{
	u16 base_lo;
	u16 sel;
	u8 always0;
	u8 flags;
	u16 base_hi;		
}__attribute__((packed));

struct idt_ptr
{
	u16 limit;
	u32 base; 
}__attribute__((packed));

extern void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
extern void idt_flush();
extern void idt_install(void);
#endif
