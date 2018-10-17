#include <idt.h>
#include <types.h>
#include <kutils.h>
#include <video.h>
#include <irq.h>
#include <isr.h>
#include <proc.h>
#include <v86mon.h>
#include <fs.h>
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void isr127();
/* Programmable interrupt controller */
#define PIC1           0x20
#define PIC1_COMMAND   PIC1
#define PIC1_OFFSET    0x20
#define PIC1_DATA      (PIC1+1)

#define PIC2           0xA0
#define PIC2_COMMAND   PIC2
#define PIC2_OFFSET    0x28
#define PIC2_DATA      (PIC2+1)

#define PIC_EOI        0x20

#define ICW1_ICW4      0x01
#define ICW1_INIT      0x10

#define PIC_WAIT() \
	do { \
		/* May be fragile */ \
		asm volatile("jmp 1f\n\t" \
		             "1:\n\t" \
		             "    jmp 2f\n\t" \
		             "2:"); \
	} while (0)

static volatile int sync_depth = 0;

#define SYNC_CLI() asm volatile("cli")
#define SYNC_STI() asm volatile("sti")

void int_disable(void) {
	/* Check if interrupts are enabled */
	uint32_t flags;
	asm volatile("pushf\n\t"
	             "pop %%eax\n\t"
	             "movl %%eax, %0\n\t"
	             : "=r"(flags)
	             :
	             : "%eax");

	/* Disable interrupts */
	SYNC_CLI();

	/* If interrupts were enabled, then this is the first call depth */
	if (flags & (1 << 9)) {
		sync_depth = 1;
	} else {
		/* Otherwise there is now an additional call depth */
		sync_depth++;
	}
}

void int_resume(void) {
	/* If there is one or no call depths, reenable interrupts */
	if (sync_depth == 0 || sync_depth == 1) {
		SYNC_STI();
	} else {
		sync_depth--;
	}
}

void int_enable(void) {
	sync_depth = 0;
	SYNC_STI();
}
void irq_ack(size_t irq_no) {
	if (irq_no >= 8) {
		outportb(PIC2_COMMAND, PIC_EOI);
	}
	outportb(PIC1_COMMAND, PIC_EOI);
}
void *irq_routines[256] = {0}; 
void install_device(int irq, u32 (*handler)(u32 r))
{
	irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}

void irq_remap()
{
	outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
	
}

void irq_install()
{
	irq_remap();
	
    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);

    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}

void isrs_install()
{
    idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);

    idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);

    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);

    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
    idt_set_gate(127, (unsigned)isr127, 0x08, 0xee); 
}
 

const char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
int iterator = 0;
extern volatile task_t * copytask;
u32 isr_handler(u32 esp)
{
	struct regs *r = (struct regs*)esp;
	
	u32 (*handler)(struct regs *r); 
	
	handler = irq_routines[r->int_no];	
	if(handler)
		handler(r);
	if(r->int_no == 7)
    {
		
    __asm__ __volatile__("CLTS"); 

 
    if (FPUTask)
    {
      
        __asm__ __volatile__("fsave (%0)" :: "r" (FPUTask->FPUptr));
    }

    FPUTask = current_task;


    if (current_task->FPUptr)
    {
     
        __asm__ __volatile__("frstor (%0)" :: "r" (current_task->FPUptr));
    }
    else
    {
       
        current_task->FPUptr = valloc(10800);
    }
return esp;
}
	/*if (r->eflags & 0x20000) 
    {
        if (vm86_opcode_handler(r))
        {
			/*printk("everything is good!");*/		
		/*}
		else
        {       			
            printk("v86: opcode error!\n");
        }
	}
	else
	{*/	

//}
if(r->int_no < 32)
	{
		//printk("Received interrupt: %d (%s)\n", r->int_no, exception_messages[r->int_no]);
		//printk("EAX=%x EBX=%x ECX=%x EDX=%x\n", r->eax, r->ebx, r->ecx, r->edx);
		//printk("ESI=%x EDI=%x EBP=%x\n", r->esi, r->edi, r->ebp);
		//printk("CS =%x EIP=%x EFLAGS=%x USERESP=%x\n", r->cs, r->eip, r->eflags, r->useresp);
		//printk("INT=%02dd ERR_CODE=0x%x DS=%x\n", r->int_no, r->err_code, r->ds);
		//printk("\n");
		sleep(2);
		uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r"(faulting_address));
	//printk("faulting_address %x\n", faulting_address);
	return esp;
			
	}
	return esp;
	
}

u32 irq_handler(u32 esp) 
{
	
	struct regs *r = (struct regs*)esp;
	
	if(task_switching)
		esp = _task_switch(esp);
task_switching = 0;
	u32 (*handler)(struct regs *r); 
	
	handler = irq_routines[r->int_no - 32];	
	if(handler)
		handler(r);
	
	if(r->int_no == 7)
    {
		
    __asm__ __volatile__("CLTS"); 

 
    if (FPUTask)
    {
      
        __asm__ __volatile__("fsave (%0)" :: "r" (FPUTask->FPUptr));
    }

    FPUTask = current_task;


    if (current_task->FPUptr)
    {
     
        __asm__ __volatile__("frstor (%0)" :: "r" (current_task->FPUptr));
    }
    else
    {
       
        current_task->FPUptr = valloc(10800);
    }
}

	   if (r->eflags & 0x20000) 
    {
        if (vm86sensitiveOpcodehandler(r)) 
        {
		
	}
		else
        {
            
           // puts("nvm86: sensitive opcode error\n");
        }
	}
	
	if(r->int_no >= 40) 
	     outb(0xA0, 0x20);
	     
	outb(0x20,0x20);

	return esp;
}
