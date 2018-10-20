#ifndef FS_H_
#define FS_H_
#include <errno.h>
#include <logging.h>
#define debug_print(level, ...) printk( __VA_ARGS__)
#define SHM_START 0xB0000000
#define IRQ_OFF int_disable()
#define IRQ_RES int_resume()
#define IRQ_ON int_enable()
#define SIGHUP      1  /* Hangup */
#define SIGINT      2  /* Interupt */
#define SIGQUIT     3  /* Quit */
#define SIGILL      4  /* Illegal instruction */
#define SIGTRAP     5  /* A breakpoint or trace instruction has been reached */
#define SIGABRT     6  /* Another process has requested that you abort */
#define SIGEMT      7  /* Emulation trap XXX */
#define SIGFPE      8  /* Floating-point arithmetic exception */
#define SIGKILL     9  /* You have been stabbed repeated with a large knife */
#define SIGBUS      10 /* Bus error (device error) */
#define SIGSEGV     11 /* Segmentation fault */
#define SIGSYS      12 /* Bad system call */
#define SIGPIPE     13 /* Attempted to read or write from a broken pipe */
#define SIGALRM     14 /* This is your wakeup call. */
#define SIGTERM     15 /* You have been Schwarzenegger'd */
#define SIGUSR1     16 /* User Defined Signal #1 */
#define SIGUSR2     17 /* User Defined Signal #2 */
#define SIGCHLD     18 /* Child status report */
#define SIGPWR      19 /* We need moar powah! */
#define SIGWINCH    20 /* Your containing terminal has changed size */
#define SIGURG      21 /* An URGENT! event (On a socket) */
#define SIGPOLL     22 /* XXX OBSOLETE; socket i/o possible */
#define SIGSTOP     23 /* Stopped (signal) */
#define SIGTSTP     24 /* ^Z (suspend) */
#define SIGCONT     25 /* Unsuspended (please, continue) */
#define SIGTTIN     26 /* TTY input has stopped */
#define SIGTTOUT    27 /* TTY output has stopped */
#define SIGVTALRM   28 /* Virtual timer has expired */
#define SIGPROF     29 /* Profiling timer expired */
#define SIGXCPU     30 /* CPU time limit exceeded */
#define SIGXFSZ     31 /* File size limit exceeded */
#define SIGWAITING  32 /* Herp */
#define SIGDIAF     33 /* Die in a fire */
#define SIGHATE     34 /* The sending process does not like you */
#define SIGWINEVENT 35 /* Window server event */
#define SIGCAT      36 /* Everybody loves cats */

#define MAXNAMLEN 256
#define SIGTTOU     37

#define NUMSIGNALS  38

#define NSIG NUMSIGNALS
 typedef  unsigned int uint32_t;
 typedef signed long int32_t;

 typedef  unsigned short uint16_t;
 typedef signed char int8_t ;
typedef unsigned char uint8_t;

typedef long int off_t;
typedef unsigned short ino_t;
struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                   by all file system types */
    char           d_name[256]; /* filename */
};

typedef struct DIR {
	int fd;
	int cur_entry;
} DIR;
#define _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_TYPE

// struct dirent.d_type flags
enum {
	DT_UNKNOWN = 0,
	DT_FIFO = 1,
	DT_CHR = 2,
	DT_DIR = 4,
	DT_BLK = 6,
	DT_REG = 8,
	DT_LNK = 10,
	DT_SOCK = 12,
    DT_WHT = 14
};
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP  ".."
#define PATH_DOT "."

#define O_RDONLY     0x0000
#define O_WRONLY     0x0001
#define O_RDWR       0x0002
#define O_APPEND     0x0008
#define O_CREAT      0x0200
#define O_TRUNC      0x0400
#define O_EXCL       0x0800
#define O_NOFOLLOW   0x1000
#define O_PATH       0x2000

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define _IFMT       0170000 /* type of file */
#define     _IFDIR  0040000 /* directory */
#define     _IFCHR  0020000 /* character special */
#define     _IFBLK  0060000 /* block special */
#define     _IFREG  0100000 /* regular */
#define     _IFLNK  0120000 /* symbolic link */
#define     _IFSOCK 0140000 /* socket */
#define     _IFIFO  0010000 /* fifo */

struct fs_node;

typedef uint32_t (*read_type_t) (struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t) (struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t) (struct fs_node *, unsigned int flags);
typedef void (*close_type_t) (struct fs_node *);
 typedef struct dirent *(*readdir_type_t) (struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t) (struct fs_node *, char *name);
typedef void (*create_type_t) (struct fs_node *, char *name, uint16_t permission);
typedef void (*unlink_type_t) (struct fs_node *, char *name);
typedef void (*mkdir_type_t) (struct fs_node *, char *name, uint16_t permission);
typedef int (*ioctl_type_t) (struct fs_node *, int request, void * argp);
typedef int (*get_size_type_t) (struct fs_node *);
typedef int (*chmod_type_t) (struct fs_node *, int mode);
typedef void (*symlink_type_t) (struct fs_node *, char * name, char * value);
typedef int (*readlink_type_t) (struct fs_node *, char * buf, size_t size);
typedef int (*selectcheck_type_t) (struct fs_node *);
typedef int (*selectwait_type_t) (struct fs_node *, void * process);
typedef int (*chown_type_t) (struct fs_node *, int, int);

typedef struct fs_node {
	char name[256];         /* The filename. */
	void * device;          /* Device object (optional) */
	uint32_t mask;          /* The permissions mask. */
	uint32_t uid;           /* The owning user. */
	uint32_t gid;           /* The owning group. */
	uint32_t flags;         /* Flags (node type, etc). */
	uint32_t inode;         /* Inode number. */
	uint32_t length;        /* Size of the file, in byte. */
	uint32_t impl;          /* Used to keep track which fs it belongs to. */
	uint32_t open_flags;    /* Flags passed to open (read/write/append, etc.) */

	/* times */
	uint32_t atime;         /* Accessed */
	uint32_t mtime;         /* Modified */
	uint32_t ctime;         /* Created  */

	/* File operations */
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	create_type_t create;
	mkdir_type_t mkdir;
	ioctl_type_t ioctl;
	get_size_type_t get_size;
	chmod_type_t chmod;
	unlink_type_t unlink;
	symlink_type_t symlink;
	readlink_type_t readlink;

	struct fs_node *ptr;   /* Alias pointer, for symlinks. */
	uint32_t offset;       /* Offset for read operations XXX move this to new "file descriptor" entry */
	int32_t refcount;
	uint32_t nlink;

	selectcheck_type_t selectcheck;
	selectwait_type_t selectwait;

	chown_type_t chown;
} fs_node_t;
/* POSIX struct dirent */

struct stat  {
	uint16_t  st_dev;
	uint16_t  st_ino;
	uint32_t  st_mode;
	uint16_t  st_nlink;
	uint16_t  st_uid;
	uint16_t  st_gid;
	uint16_t  st_rdev;
	uint32_t  st_size;
	uint32_t  st_atime;
	uint32_t  __unused1;
	uint32_t  st_mtime;
	uint32_t  __unused2;
	uint32_t  st_ctime;
	uint32_t  __unused3;
	uint32_t  st_blksize;
	uint32_t  st_blocks;
};

struct vfs_entry {
	char * name;
	fs_node_t * file;
	char * device;
	char * fs_type;
};
 
extern fs_node_t *fs_root;
extern int pty_create(void *size, fs_node_t ** fs_master, fs_node_t ** fs_slave);

int has_permission(fs_node_t *node, int permission_bit);
uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs(fs_node_t *node, unsigned int flags);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);
int mkdir_fs(char *name, uint16_t permission);
int create_file_fs(char *name, uint16_t permission);
fs_node_t *kopen(char *filename, uint32_t flags);
char *canonicalize_path(char *cwd, char *input);
fs_node_t *clone_fs(fs_node_t * source);
int ioctl_fs(fs_node_t *node, int request, void * argp);
int chmod_fs(fs_node_t *node, int mode);
int chown_fs(fs_node_t *node, int uid, int gid);
int unlink_fs(char * name);
int symlink_fs(char * value, char * name);
//int readlink_fs(fs_node_t * node, char * buf, size_t size);
int selectcheck_fs(fs_node_t * node);
int selectwait_fs(fs_node_t * node, void * process);

void vfs_install(void);
void * vfs_mount(char * path, fs_node_t * local_root);
typedef fs_node_t * (*vfs_mount_callback)(char * arg, char * mount_point);
int vfs_register(char * name, vfs_mount_callback callback);
int vfs_mount_type(char * type, char * arg, char * mountpoint);
void vfs_lock(fs_node_t * node);

/* Debug purposes only, please */
void debug_print_vfs_tree(void);

void map_vfs_directory(char *);

int make_unix_pipe(fs_node_t ** pipes);




 
#include <tree.h>
#include <types.h>
#include <fat.h>
#include <vfs.h>

#define KERNEL_STACK_SIZE 8192
typedef enum 
{
	THREAD, VM86
	
}task_type;
 
#define TASK_RUNNING (1 << 0)
#define TASK_SLEEPING (1 << 1)

#define PRIO_DEAD 99
#define PRIO_IDLE 0
#define PRIO_LOW 1
#define PRIO_HIGH 2
#define CLUSTER_SIZE 200	/* to big value gives pagefault!! */
 
typedef struct open_file {
	int count; 
	u16 dev;
	u32  ino;
	u32  _cur_ino; 
	u32 offset;
	u32 size;
	//DIR *node;
	//FILE node_[2000];
	void *data;
} open_file_t;
/* Portable image struct */
typedef struct image {
	size_t    size;        /* Image size */
	uintptr_t entry;       /* Binary entry point */
	uintptr_t heap;        /* Heap pointer */
	uintptr_t heap_actual; /* Actual heap location */
	uintptr_t stack;       /* Process kernel stack */
	uintptr_t user_stack;  /* User stack */
	uintptr_t start;
	uintptr_t shm_heap;
	volatile int lock[2];
} image_t;
 
typedef struct tree_node {
	void * value;
	list_t * children;
	
	struct tree_node * parent;
} tree_node_t;

typedef struct {
	size_t nodes;
	tree_node_t * root;
	node_t * head;
} tree_t;

 
/* Resizable descriptor table */
typedef struct descriptor_table {
	fs_node_t ** entries;
	size_t       length;
	size_t       capacity;
	size_t       refs;
} fd_table_t;
#define OPEN_MAX 12
typedef struct {
	uint32_t  signum;
	uintptr_t handler;
	regs_t registers_before;
} signal_t;
/* Signal Table */
typedef struct signal_table {
	uintptr_t functions[NUMSIGNALS+1];
} sig_table_t;
/* x86 task */
typedef struct thread {
	uintptr_t  esp; /* Stack Pointer */
	uintptr_t  ebp; /* Base Pointer */
	uintptr_t  eip; /* Instruction Pointer */

	uint8_t    fpu_enabled;
	uint8_t    fp_regs[512];

	uint8_t    padding[32]; /* I don't know */

	page_directory_t * page_directory; /* Page Directory */
} thread_t;


#include "/home/sim/Hämtningar/toaruos-master/toolchain/tarballs/newlib-1.19.0/newlib/libc/include/sys/time.h"
struct task
{
	open_file_t fdtable[OPEN_MAX];

    int priority;
    /*page_directory_t* directory;*/
    u32 time_to_run;       /* Time left on quanta*/
    int time_running;      /*Time spent running*/
    int ready_to_run;
	task_type type;
	s32 id;
	u32 ss;
	u8 privilege;
	char * wd_name;
	list_t * node_waits;
	image_t image; 
	fd_table_t * fds; /* File descriptor table */
	int user;
	int mask; 
	void* FPUptr; 
	int sleep_interrupted;
	u32 eip;
	u32 esp;
	int status;
	char *name;
	u32 kernel_stack;
fs_node_t * wd_node;
tree_node_t * tree_entry; 
	struct task* next;
	u32 state;
thread_t thread; 
thread_t signal_state;
	u32 wakeup_time;
sig_table_t signals;
int group; 	
uint8_t       finished;          /* Status indicator */
	uint8_t       started;
	uint8_t       running;
	char ** cmdline;
	struct timeval start;
	struct regs * syscall_registers; /* Registers at interrupt */
	list_t *      wait_queue;
	list_t *      shm_mappings;      /* Shared memory chunk mappings */
	list_t *      signal_queue;      /* Queued signals */
	//thread_t      signal_state;
	char *        signal_kstack;
	node_t        sched_node;
	node_t        sleep_node;
	node_t *      timed_sleep_node;
uint8_t is_tasklet;
}__attribute__((packed));

typedef struct task task_t;
extern volatile bool task_switching;
extern volatile task_t *current_task;
extern volatile task_t *ready_queue;
void _task_initialize();
extern volatile task_t* FPUTask;
void create_process(void (*process)(),task_type type,u8 privilege, int argc, char** argv);
void create_v86_task(void (*thread)());
void exit();
void task1();
s32 getpid();
//void _exit(int status);
void kill(int pid2);
//int execve(char *name, char **argv, char **env);

typedef uint8_t (*tree_comparator_t) (void *, void *);

tree_t * tree_create(void);
void tree_set_root(tree_t * tree, void * value);
void tree_node_destroy(tree_node_t * node);
void tree_destroy(tree_t * tree);
void tree_free(tree_t * tree);
tree_node_t * tree_node_create(void * value);
void tree_node_insert_child_node(tree_t * tree, tree_node_t * parent, tree_node_t * node);
tree_node_t * tree_node_insert_child(tree_t * tree, tree_node_t * parent, void * value);
tree_node_t * tree_node_find_parent(tree_node_t * haystack, tree_node_t * needle);
void tree_node_parent_remove(tree_t * tree, tree_node_t * parent, tree_node_t * node);
void tree_node_remove(tree_t * tree, tree_node_t * node);
void tree_remove(tree_t * tree, tree_node_t * node);
tree_node_t * tree_find(tree_t * tree, void * value, tree_comparator_t comparator);
void tree_break_off(tree_t * tree, tree_node_t * node);
#endif
 


