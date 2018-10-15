#include <types.h>
#include <idt.h>
#include <video.h>
#include <kutils.h>
struct idt_entry idt[256];
struct idt_ptr idtpointer;

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
{
	idt[num].base_lo = (base & 0xffff);
	idt[num].base_hi = (base >> 16) & 0xffff;
	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = flags  | 0x60;  /*usermode*/
}

void idt_install()
{
	idtpointer.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtpointer.base = (u32)&idt;	
	memset(&idt, 0, sizeof(struct idt_entry) * 256);
	idt_flush();
}
