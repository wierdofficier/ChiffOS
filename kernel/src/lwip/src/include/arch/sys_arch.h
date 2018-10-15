#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__
#include <errno.h>
#define MAX_TASKS 15
#include <types.h>
#define EWOULDBLOCK	EAGAIN	/* Operation would block */
/** @brief Semaphore structure */
typedef unsigned int tid_t;
#define KMSG_SIZE	0x1000
#define INT_SYSCALL	0x80
#define MAILBOX_SIZE	128
typedef struct sem {
	/// Resource available count
	unsigned int value;
	/// Queue of waiting tasks
	tid_t queue[MAX_TASKS];
	/// Position in queue to add a task
	unsigned int wpos;
	/// Position in queue to get a task
	unsigned int rpos;
	/// Access lock
	//spinlock_irqsave_t lock;
} sem_t;
typedef sem_t sys_mutex_t;

typedef struct
{
	sem_t		sem;
	int		valid;
} sys_sem_t;
/** @brief Wait message structure
 *
 * This message struct keeps a recipient task id and the message itself */
typedef struct {
	/// The task id of the task which is waiting for this message
	tid_t	id;
	/// The message payload
	int32_t	result;
} wait_msg_t;

#define MAILBOX_TYPES(name, type) 	\
	typedef struct mailbox_##name { \
		type buffer[MAILBOX_SIZE]; \
		int wpos, rpos; \
		sem_t mails; \
		sem_t boxes; \
	} mailbox_##name##_t;

MAILBOX_TYPES(wait_msg, wait_msg_t)
MAILBOX_TYPES(int32, int32_t)
MAILBOX_TYPES(int16, int16_t)
MAILBOX_TYPES(int8, int8_t)
MAILBOX_TYPES(uint32, uint32_t)
MAILBOX_TYPES(uint16, uint16_t)
MAILBOX_TYPES(uint8, uint8_t)
MAILBOX_TYPES(ptr, void*)
typedef struct 
{	mailbox_ptr_t	mailbox;
	int		valid;
} sys_mbox_t;

typedef tid_t		sys_thread_t;

#if SYS_LIGHTWEIGHT_PROT
#if (MAX_CORES > 1) && !defined(CONFIG_TICKLESS)
typedef uint32_t sys_prot_t;
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);
#else
typedef uint32_t sys_prot_t;

static inline sys_prot_t sys_arch_protect(void)
{
	return irq_nested_disable();
}

static inline void sys_arch_unprotect(sys_prot_t pval)
{
	irq_nested_enable(pval);
}
#endif
#endif

sys_sem_t* sys_arch_netconn_sem_get(void);
void sys_arch_netconn_sem_alloc(void);
void sys_arch_netconn_sem_free(void);
#define LWIP_NETCONN_THREAD_SEM_GET()   sys_arch_netconn_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_netconn_sem_alloc()
#define LWIP_NETCONN_THREAD_SEM_FREE()  sys_arch_netconn_sem_free()

#endif /* __ARCH_SYS_ARCH_H__ */
