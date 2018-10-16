#include <types.h>
#include <irq.h>
#include <video.h>
#include <vmmngr.h>
#include <elf.h>
#include <fat.h>
#include <proc.h>
#include <syscalls.h>
#include <kutils.h>
#include <kbd.h>
#include <vesa.h>

#include <system.h>
#include <types.h>
#include <fs.h>
#include <ext2.h>
#include <logging.h>

uint32_t fork(void);
typedef unsigned int user_t;
 uint32_t read_ext2(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
extern volatile task_t * current_task;
void halt_and_catch_fire(char * error_message, const char * file, int line, struct regs * regs) {
	IRQ_OFF;
	debug_print(ERROR, "HACF: %s", error_message);
	debug_print(ERROR, "Proc: %d", getpid());
	debug_print(ERROR, "File: %s", file);
	debug_print(ERROR, "Line: %d", line);
	if (regs) {
		debug_print(ERROR, "Registers at interrupt:");
		debug_print(ERROR, "eax=0x%x ebx=0x%x", regs->eax, regs->ebx);
		debug_print(ERROR, "ecx=0x%x edx=0x%x", regs->ecx, regs->edx);
		debug_print(ERROR, "ebp=0x%x", regs->ebp);
		debug_print(ERROR, "Error code: 0x%x",  regs->err_code);
		debug_print(ERROR, "EFLAGS:     0x%x",  regs->eflags);
		debug_print(ERROR, "User ESP:   0x%x",  regs->useresp);
		debug_print(ERROR, "eip=0x%x",          regs->eip);
	}
	debug_print(ERROR, "This process has been descheduled.");
	for(;;);
	//kexit(1);
}
#define HALT_AND_CATCH_FIRE(mesg, regs) halt_and_catch_fire(mesg, __FILE__, __LINE__, regs)
#define USER_ROOT_UID (user_t)0
#define FD_INRANGE(FD) \
	((FD) < (int)current_task->fds->length && (FD) >= 0)
#define FD_ENTRY(FD) \
	(current_task->fds->entries[(FD)])
#define FD_CHECK(FD) \
	(FD_INRANGE(FD) && FD_ENTRY(FD))

#define PTR_INRANGE(PTR) \
	((uintptr_t)(PTR) > current_task->image.entry)
#define PTR_VALIDATE(PTR) \
	ptr_validate((void *)(PTR), __func__)

static void ptr_validate(void * ptr, const char * syscall) {
	if (ptr && !PTR_INRANGE(ptr)) {
		debug_print(ERROR, "SEGFAULT: invalid pointer passed to %s. (0x%x < 0x%x)",
			syscall, (uintptr_t)ptr, current_task->image.entry);
		HALT_AND_CATCH_FIRE("Segmentation fault", NULL);
	}
}

void validate(void * ptr) {
	ptr_validate(ptr, "syscall");
}
typedef signed int pid_t;
 static int sys_sleepabs(unsigned long seconds, unsigned long subseconds) {
	/* Mark us as asleep until <some time period> */
	sleep_until((task_t *)current_task, seconds, subseconds);

	/* Switch without adding us to the queue */
	_task_switch(0);

	if (seconds > timerticks || (seconds == timerticks && subseconds >= timer_subticks)) {
		return 0;
	} else {
		return 1;
	}
}

static int sys_sleep(unsigned long seconds, unsigned long subseconds) {
	unsigned long s, ss;
	relative_time(seconds, subseconds * 10, &s, &ss);
	return sys_sleepabs(s, ss);
}

static int sys_umask(int mode) {
	current_task->mask = mode & 0777;
	return 0;
}

static int sys_unlink(char * file) {
	PTR_VALIDATE(file);
	return unlink_fs(file);
}

static int sys_fork(void) {
	return (int)fork();
}

static int sys_clone(uintptr_t new_stack, uintptr_t thread_func, uintptr_t arg) {
	if (!new_stack || !PTR_INRANGE(new_stack)) return -1;
	if (!thread_func || !PTR_INRANGE(thread_func)) return -1;
	return (int)clone(new_stack, thread_func, arg);
}

static int sys_shm_obtain(char * path, size_t * size) {
	PTR_VALIDATE(path);
	PTR_VALIDATE(size);

	return (int)shm_obtain(path, size);
}

static int sys_shm_release(char * path) {
	PTR_VALIDATE(path);

	return shm_release(path);
}

static int sys_kill(pid_t process, uint32_t signal) {
	return send_signal(process, signal);
}

static int sys_gettimeofday(struct timeval * tv, void * tz) {
	PTR_VALIDATE(tv);
	PTR_VALIDATE(tz);

	return gettimeofday(tv, tz);
}
static int sys_open(const char * file, int flags, int mode) {
	PTR_VALIDATE(file);
	debug_print(NOTICE, "open(%s) flags=0x%x; mode=0x%x", file, flags, mode);
 
	fs_node_t * node = kopen2((char *)file, flags);

	if (node && !has_permission(node, 04)) {
		debug_print(WARNING, "access denied (read, sys_open, file=%s)", file);
		return -EACCES;
	}
	if (node && ((flags & O_RDWR) || (flags & O_APPEND) || (flags & O_WRONLY))) {
		if (!has_permission(node, 02)) {
			debug_print(WARNING, "access denied (write, sys_open, file=%s)", file);
			return -EACCES;
		}
	}

	if (!node && (flags & O_CREAT)) {
		/* TODO check directory permissions */
		debug_print(NOTICE, "- file does not exist and create was requested.");
		/* Um, make one */
		int result = create_file_fs((char *)file, mode);
		if (!result) {
			node = kopen2((char *)file, flags);
		} else {
			return result;
		}
	}
	if (!node) {
		debug_print(NOTICE, "File does not exist; someone should be setting errno?");
		return -1;
	}
	node->offset = 0;
 
	int fd = process_append_fd((task_t *)current_task, node);
	debug_print(INFO, "[open] pid=%d %s -> %d", getpid(), file, fd);
	return fd ;
}
static int sys_openpty(int * master, int * slave, char * name, void * _ign0, void * size) {
	/* We require a place to put these when we are done. */
	if (!master || !slave) return -1;
	if (master && !PTR_INRANGE(master)) return -1;
	if (slave && !PTR_INRANGE(slave)) return -1;
	if (size && !PTR_INRANGE(size)) return -1;

	/* Create a new pseudo terminal */
	fs_node_t * fs_master;
	fs_node_t * fs_slave;

	pty_create(size, &fs_master, &fs_slave);

	/* Append the master and slave to the calling process */
	*master = process_append_fd((task_t *)current_task, fs_master);
	*slave  = process_append_fd((task_t *)current_task, fs_slave);

	open_fs(fs_master, 0);
	open_fs(fs_slave, 0);

	/* Return success */
	return 0;
}

static int sys_pipe(int pipes[2]) {
	if (pipes && !PTR_INRANGE(pipes)) {
		return -EFAULT;
	}

	fs_node_t * outpipes[2];

	make_unix_pipe(outpipes);

	open_fs(outpipes[0], 0);
	open_fs(outpipes[1], 0);

	pipes[0] = process_append_fd((task_t *)current_task, outpipes[0]);

	pipes[1] = process_append_fd((task_t*)current_task, outpipes[1]);

	return 0;
}

static int sys_mount(char * arg, char * mountpoint, char * type, unsigned long flags, void * data) {
	/* TODO: Make use of flags and data from mount command. */
	(void)flags;
	(void)data;

	if (current_task->user != USER_ROOT_UID) {
		return -EPERM;
	}

	if (PTR_INRANGE(arg) && PTR_INRANGE(mountpoint) && PTR_INRANGE(type)) {
		return vfs_mount_type(type, arg, mountpoint);
	}

	return -EFAULT;
}

static int sys_symlink(char * target, char * name) {
	PTR_VALIDATE(target);
	PTR_VALIDATE(name);
	return symlink_fs(target, name);
}

static int sys_readlink(const char * file, char * ptr, int len) {
	PTR_VALIDATE(file);
	fs_node_t * node = kopen2((char *) file, O_PATH | O_NOFOLLOW);
	if (!node) {
		return -ENOENT;
	}
	int rv = readlink_fs(node, ptr, len);
	close_fs(node);
	return rv;
}

static int sys_lstat(char * file, uintptr_t st) {
	int result;
	PTR_VALIDATE(file);
	PTR_VALIDATE(st);
	fs_node_t * fn = kopen2(file, O_PATH | O_NOFOLLOW);
	result = stat_node(fn, st);
	if (fn) {
		close_fs(fn);
	}
	return result;
}

static int sys_fswait(int c, int fds[]) {
	PTR_VALIDATE(fds);
	for (int i = 0; i < c; ++i) {
		if (!FD_CHECK(fds[i])) return -1;
	}
	fs_node_t ** nodes = malloc(sizeof(fs_node_t *)*(c+1));
	for (int i = 0; i < c; ++i) {
		nodes[i] = FD_ENTRY(fds[i]);
	}
	nodes[c] = NULL;

	int result = process_wait_nodes((task_t *)current_task, nodes, -1);
	free(nodes);
	return result;
}

static int sys_fswait_timeout(int c, int fds[], int timeout) {
	PTR_VALIDATE(fds);
	for (int i = 0; i < c; ++i) {
		if (!FD_CHECK(fds[i])) return -1;
	}
	fs_node_t ** nodes = malloc(sizeof(fs_node_t *)*(c+1));
	for (int i = 0; i < c; ++i) {
		nodes[i] = FD_ENTRY(fds[i]);
	}
	nodes[c] = NULL;

	int result = process_wait_nodes((task_t *)current_task, nodes, timeout);
	free(nodes);
	return result;
}


  int stat_node(fs_node_t * fn, uintptr_t st) {
	struct stat * f = (struct stat *)st;

	PTR_VALIDATE(f);

	if (!fn) {
		memset(f, 0x00, sizeof(struct stat));
		debug_print(INFO, "stat: This file doesn't exist");
		return -1;
	}
	f->st_dev   = (uint16_t)(((uint32_t)fn->device & 0xFFFF0) >> 8);
	f->st_ino   = fn->inode;

	uint32_t flags = 0;
	if (fn->flags & FS_FILE)        { flags |= _IFREG; }
	if (fn->flags & FS_DIRECTORY)   { flags |= _IFDIR; }
	if (fn->flags & FS_CHARDEVICE)  { flags |= _IFCHR; }
	if (fn->flags & FS_BLOCKDEVICE) { flags |= _IFBLK; }
	if (fn->flags & FS_PIPE)        { flags |= _IFIFO; }
	if (fn->flags & FS_SYMLINK)     { flags |= _IFLNK; }

	f->st_mode  = fn->mask | flags;
	f->st_nlink = fn->nlink;
	f->st_uid   = fn->uid;
	f->st_gid   = fn->gid;
	f->st_rdev  = 0;
	f->st_size  = fn->length;

	f->st_atime = fn->atime;
	f->st_mtime = fn->mtime;
	f->st_ctime = fn->ctime;
	f->st_blksize = 512; /* whatever */

	if (fn->get_size) {
		f->st_size = fn->get_size(fn);
	}

	return 0;
}

  int sys_write(int fd, char * ptr, int len) {
  printk("%s",ptr );
	if (FD_CHECK(fd)) {
		PTR_VALIDATE(ptr);
		fs_node_t * node = FD_ENTRY(fd);
		if (!has_permission(node, 02)) {
			debug_print(WARNING, "access denied (write, fd=%d)", fd);
			return -EACCES;
		}
		uint32_t out = write_fs(node, node->offset, len, (uint8_t *)ptr);
		printk("%s\n",(uint8_t *)ptr); //ändring
		node->offset += out;
		return out;
	}
	return -1;
}
  int sys_access(const char * file, int flags) {
	PTR_VALIDATE(file);
	debug_print(INFO, "access(%s, 0x%x) from pid=%d", file, flags, getpid());
	fs_node_t * node = kopen2((char *)file, 0);
	if (!node) return -1;
	close_fs(node);
	return 0;
}
 void nop()
{
__asm__ __volatile__ ("nop");	
}
void reboot_old()
{
outb (0x64, 0xFE);
}

u32 ino = 0;
int fstat(int fd, struct stat* st) {
    printk( "SYSCALL : fstat(%d, %p)\n", fd, st);
	
    //if ( st == NULL ) return -1;
  //  st->st_dev = 0;
  //  st->st_ino = 0;
    st->st_mode = 0100000;
    st->st_mtime = 0;
    st->st_ino = 1;
  //  st->st_nlink = 0;
  //  st->st_uid = 0;
  //  st->st_gid = 0;
  //  st->st_rdev = 0;
  //if(fd != 1)
   st->st_size = current_task->image.size ;//current_task->files->fd[fd]->f_inode->i_size;

  //  st->st_atime =0;
  //  st->st_mtime =0;
  //  st->st_ctime =0;  
    printk("st->st_ino %d\n",st->st_ino);
     printk("st->st_size %d\n",st->st_size);
      
                           //    st->st_blksize = 1;
    return 0;
}   

extern struct page_directory * current_directory;
  int sys_sbrk(int size) {

	task_t * proc = (task_t *)current_task;

	if (proc->group != 0) {

		proc = process_from_pid(proc->group);
	}

	//spin_lock(proc->image.lock);
	uintptr_t ret = proc->image.heap;
	uintptr_t i_ret = ret;
	ret = (ret + 0xfff) & ~0xfff; /* Rounds ret to 0x1000 in O(1) */
	proc->image.heap += (ret - i_ret) + size;

	while (proc->image.heap > proc->image.heap_actual) {

		proc->image.heap_actual += 0x1000;
		//assert(proc->image.heap_actual % 0x1000 == 0);
		alloc_frame(get_page(proc->image.heap_actual, 1, current_directory), 0, 1);

		invalidate_tables_at(proc->image.heap_actual);
	}
	//spin_unlock(proc->image.lock);

	return ret;
}
 
 

  int sys_stat(int fd, uintptr_t st) {
  
	 PTR_VALIDATE(st);
	// printk( "SYSCALL : sys_stat(%x) \n",  current_task->fds->entries[1] );
	 if (FD_CHECK(fd)) {
 
		return stat_node(FD_ENTRY(fd), st);
 	}
	return -1;
}
static int sys_seek(int fd, int offset, int whence) {
	if (FD_CHECK(fd)) {
		if (fd < 3) {
			return 0;
		}
		switch (whence) {
			case 0:
				FD_ENTRY(fd)->offset = offset;
				break;
			case 1:
				FD_ENTRY(fd)->offset += offset;
				break;
			case 2:
				FD_ENTRY(fd)->offset = FD_ENTRY(fd)->length + offset;
				break;
		}
		return FD_ENTRY(fd)->offset;
	}
	return -1;
}
#define MIN(A, B) ((A) < (B) ? (A) : (B))
  int sys_getcwd(char * buf, size_t size) {
	if (buf) {
		PTR_VALIDATE(buf);
		size_t len = strlen(current_task->wd_name) + 1;
		return (int)memcpy(buf, current_task->wd_name, MIN(size, len));
	}
	return 0;
}
u8 getchar();
static int sys_getuid(void) {
	return current_task->user;
}
extern int yy;
extern int xxx;
extern unsigned short char_width;
 int USING_STDIO = 0;
int stdio_read__(int fd, void *buf, size_t length) {
	 
USING_STDIO = 1;
	char *p = (char *)buf;

	if (fd != 0) {
		return 0; // Don't allow reading from stdout/stderr
	}

	int ret = 0;
int n = 0;
	while (p < (char *)buf + length - 1 /* NULL termination */) {
		char c = getchar();

		if (c >= ' ' || c == '\n') {
			n+=char_width;
			write_char(xxx+n, yy, c, 0x11ffff00);
			 
			//putch(c); // echo to screen
			//update_cursor();
		}
		else if (c == '\b') {
			if (p > (char *)buf) {
				p--;
				n+=char_width;
				write_char(xxx+n, yy, c, 0x11ffff00);
			//	putch(c);
			//	putch(' ');
			//	putch(c);
				//update_cursor();
				ret--;
			}
		}
		else if (c == -1) {
			// EOF sent by Ctrl-D
			if (ret > 0)
				continue;
			else {
				//putch('^');
				//putch('D');
				return 0;
			}
		}

		if (c == '\r' || c == '\n') {
			ret++;
			*p++ = c;
			*p = 0;
			return ret;
		}
		else if (c != '\b') {
			*p++ = c;
			ret++;
		}
	}

	assert(p < (char *)buf + length);
	*p = 0;

	return ret;
}
static int sys_read(int fd, char * ptr, int len) {
 //puts_g( ptr);
 //
if(fd == 0)
{
 	int ret = stdio_read__(fd, ptr,len);
	return ret;
}
 
	if (FD_CHECK(fd)) {
		PTR_VALIDATE(ptr);
 
		fs_node_t * node = FD_ENTRY(fd);
 		 //node->read = read_ext2 ;
 
  		 // printk("node->offset %d :: len = %d\n",node->offset,len);
		
		uint32_t out = read_fs(node, node->offset, len, (uint8_t *)ptr);
	    
		node->offset += out;
		return (int)out;
	}
 
	return -1;
}
static int sys_signal(uint32_t signum, uintptr_t handler) {
	if (signum > NUMSIGNALS) {
		return -1;
	}
	uintptr_t old = current_task->signals.functions[signum];
	current_task->signals.functions[signum] = handler;
	return (int)old;
}
 

static int sys_ioctl(int fd, int request, void * argp) {
	if (FD_CHECK(fd)) {
		PTR_VALIDATE(argp);
		return ioctl_fs(FD_ENTRY(fd), request, argp);
	}
	return -1;
}
static int sys_gettid(void) {
	return getpid();
}

static int sys_close(int fd) {
	if (FD_CHECK(fd)) {
		close_fs(FD_ENTRY(fd));
		FD_ENTRY(fd) = NULL;
		return 0;
	}
	return -1;
}

 static int sys_waitpid(int pid, int * status, int options) {
	if (status && !PTR_INRANGE(status)) {
		return -EINVAL;
	}
	return waitpid(pid, status, options);
}
static int sys_readdir(int fd, int index, struct dirent * entry) {
	if (FD_CHECK(fd)) {

		PTR_VALIDATE(entry);
		struct dirent * kentry = readdir_fs(FD_ENTRY(fd), (uint32_t)index);
		if (kentry) {
			memcpy(entry, kentry, sizeof *entry);
			free(kentry);
			return 0;
		} else {
			return 1;
		}
	}
	return -1;
}

static int sys_yield(void) {
	_task_switch(1);
	return 1;
}
static int sys_chdir(char * newdir) {
	PTR_VALIDATE(newdir);
	char * path = canonicalize_path(current_task->wd_name, newdir);
	fs_node_t * chd = kopen(path, 0);
	if (chd) {
		if ((chd->flags & FS_DIRECTORY) == 0) {
			close_fs(chd);
			return -1;
		}
		close_fs(chd);
		free(current_task->wd_name);
		current_task->wd_name = malloc(strlen(path) + 1);
		memcpy(current_task->wd_name, path, strlen(path) + 1);
		return 0;
	} else {
		return -1;
	}
}



static int sys_execve(const char * filename, char *const argv[], char *const envp[]) {
	PTR_VALIDATE(argv);
	PTR_VALIDATE(filename);
	PTR_VALIDATE(envp);

	debug_print(NOTICE, "%d = exec(%s, ...)", current_task->id, filename);

	int argc = 0;
	int envc = 0;
	while (argv[argc]) {
		PTR_VALIDATE(argv[argc]);
		++argc;
	}

	if (envp) {
		while (envp[envc]) {
			PTR_VALIDATE(envp[envc]);
			++envc;
		}
	}

	debug_print(INFO, "Allocating space for arguments...");
	char ** argv_ = malloc(sizeof(char *) * (argc + 1));
	for (int j = 0; j < argc; ++j) {
		argv_[j] = malloc((strlen(argv[j]) + 1) * sizeof(char));
		memcpy(argv_[j], argv[j], strlen(argv[j]) + 1);
	}
	argv_[argc] = 0;
 
	char ** envp_;
	/*if (envp && envc) {
		envp_ = malloc(sizeof(char *) * (envc + 1));
		for (int j = 0; j < envc; ++j) {
			envp_[j] = malloc((strlen(envp[j]) + 1) * sizeof(char));
			memcpy(envp_[j], envp[j], strlen(envp[j]) + 1);
		}
		envp_[envc] = 0;
	} else {
		envp_ = malloc(sizeof(char *));
		envp_[0] = NULL;
	}*/
	debug_print(INFO,"Releasing all shmem regions...");
	 shm_release_all((task_t *)current_task);

	current_task->cmdline = argv_;

	debug_print(INFO,"Executing...");
	/* Discard envp */
	exec((char *)filename, argc, (char **)argv_, (char **)NULL/*envp_*/);
	return -1;
}
 typedef unsigned int socklen_t;
 int getsockname(int s, struct sockaddr *name, socklen_t *namelen);
int gettimeofday(struct timeval *p, void *z);
int bind(int s, const struct sockaddr *name, socklen_t namelen);
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
int connect(int s, const struct sockaddr *name, socklen_t namelen);
int recv(int s, void *mem, size_t len, int flags);
int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int send(int s, const void *dataptr, size_t size, int flags);
int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int socket(int domain, int type, int protocol);
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
int fcntl(int s, int cmd, int val);
int shutdown(int socket, int how);
int listen(int s, int backlog);
void sleep2(u32 milliseconds);
unsigned int ipaddr_addr(const char *cp);

void * sbrk(uintptr_t increment);
 void* syscalls[] =
{
&exit,			//0
&nop,
&sys_open,
&sys_read,
&sys_write,
&sys_close, 			//5
&gettimeofday,
&sys_execve,
&sys_fork,
&sys_gettid,
&sys_sbrk, 		//10
&nop,
&nop,
&nop,
&sys_seek,
&sys_lstat, 			//15
&nop,
&nop,
&nop,
&nop,
&nop, 			//20
&nop,
&nop,
&sys_getuid,
&nop,
&nop,	//25
&nop,
&sys_readdir,
&sys_chdir,
&sys_getcwd,
&nop,
&nop,
&nop,			//32
&nop,
&nop,
&nop,
&nop,
&sys_kill,
&sys_signal,
&nop,
&nop,			//40
&nop,
&sys_yield,
&nop,
&nop,
&nop,
&sleep2,
&sys_ioctl,
&sys_access,
&nop,
&nop,
&nop,
&nop,
&sys_waitpid,
&sys_pipe, 		//54
&nop,
&nop,
&nop,
&nop,			
&nop,
&nop,
&ipaddr_addr,
&bind,
&nop,
&setsockopt,
&nop,
&recv,
&select,
&accept,
&getsockname,
&nop,
&getpeername,   //71
&connect,
&nop,
&listen,
&nop,
&nop,
&nop,
&socket,
&send,
&nop
};
 
 
void syscall_handler(struct regs *r)
{
	//   printk("regs nr: %d\n",r->eax);
	    if (r->eax >= sizeof(syscalls)/sizeof(*syscalls))
		return;

	  	/* Update the syscall registers for this process */
	    current_task->syscall_registers = r;

	    void* addr = syscalls[r->eax];

	    __asm__ __volatile__(
	     "push %1; \
	      push %2; \
	      push %3; \
	      push %4; \
	      push %5; \
	      call *%6; \
	      add $20, %%esp;"
	       : "=a" (r->eax) : "D" (r->edi), "S" (r->esi), "d" (r->edx), "c" (r->ecx), "b" (r->ebx), "a" (addr));   	
}

void _install_syscall_handler()
{
	install_device(0x7F,syscall_handler);
}
