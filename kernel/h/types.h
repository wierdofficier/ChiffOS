#ifndef TYPES_H_
#define TYPES_H_


typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef unsigned int       uint32_t;

 typedef unsigned __PTRDIFF_TYPE__ uintptr_t;

typedef long int		fat32;
typedef	fat32		status_t; 
void outportb(u16 port, u8 data);
#ifndef __bool_true_false_are_defined
  typedef _Bool             bool;
  #define true  1
  #define false 0
  #define __bool_true_false_are_defined 1
#endif
typedef int			 size_t;

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
extern void panic_assert(const char *file, u32 line, const char *desc);
struct regs
{
    unsigned int gs, fs, es, ds;      
    unsigned int  edi, esi, ebp, ebx, edx, ecx, eax;  
    unsigned int int_no, err_code;    
    unsigned int eip, cs, eflags, useresp,ss;  
}__attribute__((packed));
struct regs__
{
  unsigned int gs, fs, es, ds;      
    unsigned int  edi, esi, ebp, ebx, edx, ecx, eax;  
    unsigned int int_no, err_code;    
    unsigned int eip, cs, eflags, useresp,ss; 
}__attribute__((packed));

typedef struct regs regs_t;
#define NULL ((void*)0)
#define	EFAULT		14	/* Bad address */
#define   ENOSYS          38
#define   ENODEV          1
 #define	EINVAL		22	/* Invalid argument */
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
#define BIT(n) (1<<(n))
#endif 
