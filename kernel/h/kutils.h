#ifndef KUTIlS_H_
#define KUTIlS_H_
#include <types.h>

#include <stdint.h>
void outb(u16 port, u8 data);
u8 inb(u16 port);
int strlen( char* str);
u16 *memsetw(u16 *dest, u16 value, int count);
void *memcpy(void *dest,const void *src,size_t n);
void *memset(void *dest,int val,size_t n);
void ToDosFileName (const char* filename,
             char* fname,
            unsigned int FNameLength);
u32 strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
void reboot();
#endif
