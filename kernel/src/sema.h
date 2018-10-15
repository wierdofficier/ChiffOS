#ifndef SEMA_H
#define SEMA_H
typedef struct sys_sem_attr
{
unsigned int attr_protocol;
unsigned int  attr_pshared;
unsigned int  key;
unsigned int  flags;
unsigned int  pad;
char name[8];
} sys_sem_attr_t;
extern void syscall_sem_create(unsigned int *phandle, sys_sem_attr_t *attr, int initialValue, int SEM_VALUE_MAX);
extern void syscall_sysSemPost(void *handle, int count);
extern void syscall_sys_sem_free(void * sem);
extern unsigned int syscall_sys_arch_sem_wait(void *sem, unsigned int timeout);
extern unsigned int syscall_sys_arch_sem_trywait(void * sem);
#endif
