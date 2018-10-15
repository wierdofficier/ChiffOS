#ifndef VM86_H
#define VM86_H

#include <types.h>

#define VALID_FLAGS  0x3FFFFF
#define EFLAG_IF     BIT(9)
#define EFLAG_VM     BIT(17)
#define FP_TO_LINEAR(seg, off) ((void*) ((((u16) (seg)) << 4) + ((u16) (off))))

typedef struct
{
    u32       v86_if;
    u32       v86_in_handler;
    u32       kernel_esp;
    u32       user_stack_top;
    u32       v86_handler;
} current_t;

bool vm86_opcode_handler(regs_t *ctx);

#endif
