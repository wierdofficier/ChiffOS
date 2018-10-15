#ifndef KBD_H
#define KBD_H
#include <types.h>

typedef struct ringbuffer {
 volatile u8 data[256];
 volatile u8 *read_ptr;
 volatile u8 *write_ptr; /* volatile is probably not needed */
 volatile u32 counter; /* how much unread data is stored? */
} ringbuffer_t;
void _kbd_initialize();
//u8 getchar();
#endif
