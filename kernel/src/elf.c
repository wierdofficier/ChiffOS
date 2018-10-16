#include <system.h>
#include <fs.h>
 
#include <elf.h>
#include <process.h>
#include <logging.h>
#define USER_STACK_BOTTOM 0xAFF00000
#define USER_STACK_TOP    0xB0000000
#define SHM_START 0xB0000000
#define PUSH(stack, type, item) stack -= sizeof(type); \
*((type *) stack) = item
extern list_t * process_list;  
  task_t* new_task;
void
enter_user_jmp(uintptr_t location, int argc, char ** argv, uintptr_t stack) {
	//IRQ_OFF;

	set_kernel_stack(new_task->image.stack);

	PUSH(stack, uintptr_t, (uintptr_t)argv);
	PUSH(stack, int, argc);
  create_user_task(location, new_task,argc,argv);
	// enter_userspace(location, stack);
}

 extern unsigned char  * lfb_vid_memory;
extern struct page_directory * current_directory;

u32 heap_actual_global;
u32 heap_global;
 
extern u32 volatile pid ;
 
extern tree_t * process_tree;
#define KERNEL_STACK_SIZE  4096
extern int DOTASKSWITCH;
 volatile task_t * copytask;
int exec_elf(char * path, fs_node_t * file, int argc, char ** argv, char ** env, int interp) {
	Elf32_Header header;

	read_fs(file, 0, sizeof(Elf32_Header), (uint8_t *)&header);
	 
	//assert(directory && "Could not allocate a new page directory!");
	/* Spawn a new process from this one */
	   
	
if (header.e_ident[0] != ELFMAG0 ||
	    header.e_ident[1] != ELFMAG1 ||
	    header.e_ident[2] != ELFMAG2 ||
	    header.e_ident[3] != ELFMAG3) {
		debug_print(ERROR, "Not a valid ELF executable.");
		close_fs(file);
		return -1;
	}
	printk("entry = %x \n", (uintptr_t)header.e_entry);
	if (file->mask & 0x800) {
		debug_print(WARNING, "setuid binary executed [%s, uid:%d]", file->name, file->uid);
		new_task->user = file->uid;
	}

	for (uintptr_t x = 0; x < (uint32_t)header.e_phentsize * header.e_phnum; x += header.e_phentsize) {
		Elf32_Phdr phdr;
		read_fs(file, header.e_phoff + x, sizeof(Elf32_Phdr), (uint8_t *)&phdr);
		if (phdr.p_type == PT_DYNAMIC) {
			/* Dynamic */
			close_fs(file);

			/* Find interpreter? */
			debug_print(WARNING, "Dynamic executable");

			unsigned int nargc = argc + 3;
			char * args[nargc+1];
			args[0] = "ld.so";
			args[1] = "-e";
			args[2] = strdup(new_task->name);
			int j = 3;
			for (int i = 0; i < argc; ++i, ++j) {
				args[j] = argv[i];
			}
			args[j] = NULL;

			fs_node_t * file = kopen("/lib/ld.so",0);
			if (!file) return -1;

			return exec_elf(NULL, file, nargc, args, env, 1);
		}
	}

	uintptr_t entry = (uintptr_t)header.e_entry;
	uintptr_t base_addr = 0xFFFFFFFF;
	uintptr_t end_addr  = 0x0;

	for (uintptr_t x = 0; x < (uint32_t)header.e_phentsize * header.e_phnum; x += header.e_phentsize) {
		Elf32_Phdr phdr;
		read_fs(file, header.e_phoff + x, sizeof(Elf32_Phdr), (uint8_t *)&phdr);
		if (phdr.p_type == PT_LOAD) {
			if (phdr.p_vaddr < base_addr) {
				base_addr = phdr.p_vaddr;
			}
			if (phdr.p_memsz + phdr.p_vaddr > end_addr) {
				end_addr = phdr.p_memsz + phdr.p_vaddr;
			}
		}
	}

	new_task->image.entry = base_addr;
	new_task->image.size  = end_addr - base_addr;

	release_directory_for_exec(current_directory);
	invalidate_page_tables();
 

	for (uintptr_t x = 0; x < (uint32_t)header.e_phentsize * header.e_phnum; x += header.e_phentsize) {
		Elf32_Phdr phdr;
		read_fs(file, header.e_phoff + x, sizeof(Elf32_Phdr), (uint8_t *)&phdr);
		if (phdr.p_type == PT_LOAD) {
			for (uintptr_t i = phdr.p_vaddr; i < phdr.p_vaddr + phdr.p_memsz; i += 0x1000) {
				/* This doesn't care if we already allocated this page */
				alloc_frame(get_page(i, 1, current_directory), 0, 1);
				invalidate_tables_at(i);
			}
			IRQ_RES;
			read_fs(file, phdr.p_offset, phdr.p_filesz, (uint8_t *)phdr.p_vaddr);
			IRQ_OFF;
			size_t r = phdr.p_filesz;
			while (r < phdr.p_memsz) {
				*(char *)(phdr.p_vaddr + r) = 0;
				r++;
			}
		}
	}

	 close_fs(file);

	for (uintptr_t stack_pointer = USER_STACK_BOTTOM; stack_pointer < USER_STACK_TOP; stack_pointer += 0x1000) {
		alloc_frame(get_page(stack_pointer, 1, current_directory), 0, 1);
		invalidate_tables_at(stack_pointer);
	}
	//__asm__ __volatile__("cli");
	

	//memset((task_t *)new_task, 0, sizeof(task_t));
	new_task->id = pid++;
    new_task->esp = 0;
    new_task->eip = 0;
    new_task->privilege = 0;
    new_task->state = TASK_RUNNING;
    new_task->next = 0;
    new_task->type = THREAD;
    new_task->priority = PRIO_HIGH;
    new_task->time_to_run = 10;
    new_task->ready_to_run = 1;
	new_task->wd_name = strdup("/");
	new_task->kernel_stack = (u32)valloc(KERNEL_STACK_SIZE)+KERNEL_STACK_SIZE;
 

	/* Allocate space for a new process */
	 
	/* Set it as the root process */
	tree_set_root(process_tree, (void *)new_task);
	/* Set its tree entry pointer so we can keep track
	 * of the process' entry in the process tree. */
	new_task->tree_entry = process_tree->root;
	new_task->id      = 1;       /* Init is PID 1 */
	new_task->group   = 0;
	new_task->name    = strdup("init");  /* Um, duh. */
	new_task->cmdline = NULL;
	new_task->user    = 0;       /* UID 0 */
	new_task->mask    = 022;     /* umask */
	new_task->group   = 0;       /* Task group 0 */
	//new_task->status  = 0;       /* Run status */
	new_task->fds = malloc(sizeof(fd_table_t));
	new_task->fds->refs = 1;
	new_task->fds->length   = 12;  /* Initialize the file descriptors */
	new_task->fds->capacity = 44;
	new_task->fds->entries  = malloc(sizeof(fs_node_t *) * new_task->fds->capacity);
	new_task->fds->entries[0] =  clone_fs(fs_root);
	/* Set the working directory */
	 new_task->wd_node = clone_fs(fs_root);
	new_task->wd_name = strdup("/");

	/* Heap and stack pointers (and actuals) */
	new_task->image.entry       = 0;
	new_task->image.heap        = 0;
	new_task->image.heap_actual = 0;
	//new_task->image.stack       = initial_esp + 1;
	new_task->image.user_stack  = 0;
	new_task->image.size        = 0;
	new_task->image.shm_heap    = SHM_START; /* Yeah, a bit of a hack. */

	//spin_init(new_task->image.lock);

	/* Process is not finished */
	new_task->finished = 0;
	new_task->started = 1;
	new_task->running = 1;
	new_task->wait_queue = list_create();
	new_task->shm_mappings = list_create();
	new_task->signal_queue = list_create();
	new_task->signal_kstack = NULL; /* None yet initialized */

	new_task->sched_node.prev = NULL;
	new_task->sched_node.next = NULL;
	//new_task->sched_node.value = init;

	new_task->sleep_node.prev = NULL;
	new_task->sleep_node.next = NULL;
	//new_task->sleep_node.value = init;

	new_task->timed_sleep_node = NULL;

	new_task->is_tasklet = 0;
 tree_node_create(new_task);
	// set_process_environment(new_task, current_directory);

	/* What the hey, let's also set the description on this one */
	//new_task->description = strdup("[init]");
	
	/* Collect arguments */
	int envc = 0;
	for (envc = 0; env[envc] != NULL; ++envc);

	/* Format auxv */
	Elf32_auxv auxv[] = {
		{256, 0xDEADBEEF},
		{0, 0}
	};
	int auxvc = 0;
	for (auxvc = 0; auxv[auxvc].id != 0; ++auxvc);
	auxvc++;


	uintptr_t heap = new_task->image.entry + new_task->image.size;
	while (heap & 0xFFF) heap++;
	alloc_frame(get_page(heap, 1, current_directory), 0, 1);
	invalidate_tables_at(heap);
	char ** argv_ = (char **)heap;
	heap += sizeof(char *) * (argc + 1);
	char ** env_ = (char **)heap;
	heap += sizeof(char *) * (envc + 1);
	void * auxv_ptr = (void *)heap;
	heap += sizeof(Elf32_auxv) * (auxvc);

	for (int i = 0; i < argc; ++i) {
		size_t size = strlen(argv[i]) * sizeof(char) + 1;
		for (uintptr_t x = heap; x < heap + size + 0x1000; x += 0x1000) {
			alloc_frame(get_page(x, 1, current_directory), 0, 1);
		}
		invalidate_tables_at(heap);
		argv_[i] = (char *)heap;
		memcpy((void *)heap, argv[i], size);
		heap += size;
	}
	/* Don't forget the NULL at the end of that... */
	argv_[argc] = 0;

	for (int i = 0; i < envc; ++i) {
		size_t size = strlen(env[i]) * sizeof(char) + 1;
		for (uintptr_t x = heap; x < heap + size + 0x1000; x += 0x1000) {
			alloc_frame(get_page(x, 1, current_directory), 0, 1);
		}
		invalidate_tables_at(heap);
		env_[i] = (char *)heap;
		memcpy((void *)heap, env[i], size);
		heap += size;
	}
	env_[envc] = 0;

	memcpy(auxv_ptr, auxv, sizeof(Elf32_auxv) * (auxvc));

	new_task->image.heap        = heap; /* heap end */
	new_task->image.heap_actual = heap + (0x1000 - heap % 0x1000);
	alloc_frame(get_page(new_task->image.heap_actual, 1, current_directory), 0, 1);
	invalidate_tables_at(new_task->image.heap_actual);
	new_task->image.user_stack  = USER_STACK_TOP;

	new_task->image.start = entry;

 
	//copytask = _task_initialize_random(copytask);
 
	/* Go go go */
 	printk("entry = %x \n", (uintptr_t)header.e_entry);
   list_insert(process_list, (void *)new_task);
 	make_process_ready(new_task);
printk("lfb_vid_memory= %x \n", (uintptr_t)lfb_vid_memory);
 
 enter_user_jmp(entry, argc, argv_, USER_STACK_TOP);
	/* We should never reach this code */
	return 0;
}
 
int exec_shebang(char * path, fs_node_t * file, int argc, char ** argv, char ** env, int interp) {
	/* Read MAX_LINE... */
	char tmp[100];
	read_fs(file, 0, 100, (unsigned char *)tmp); close_fs(file);
	char * cmd = (char *)&tmp[2];
	char * space_or_linefeed = strpbrk(cmd, " \n");
	char * arg = NULL;

	if (!space_or_linefeed) {
		debug_print(WARNING, "No space or linefeed found.");
		return -ENOEXEC;
	}

	if (*space_or_linefeed == ' ') {
		/* Oh lovely, an argument */
		*space_or_linefeed = '\0';
		space_or_linefeed++;
		arg = space_or_linefeed;
		space_or_linefeed = strpbrk(space_or_linefeed, "\n");
		if (!space_or_linefeed) {
			debug_print(WARNING, "Argument exceeded maximum length");
			return -ENOEXEC;
		}
	}
	*space_or_linefeed = '\0';

	char script[strlen(path)+1];
	memcpy(script, path, strlen(path)+1);

	unsigned int nargc = argc + (arg ? 2 : 1);
	char * args[nargc + 1];
	args[0] = cmd;
	args[1] = arg ? arg : script;
	args[2] = arg ? script : NULL;
	args[3] = NULL;

	int j = arg ? 3 : 2;
	for (int i = 1; i < argc; ++i, ++j) {
		args[j] = argv[i];
	}
	args[j] = NULL;

	return exec(cmd, nargc, args, env);
}

/* Consider exposing this and making it a list so it can be extended ... */
typedef int (*exec_func)(char * path, fs_node_t * file, int argc, char ** argv, char ** env, int interp);
typedef struct {
	exec_func func;
	unsigned char bytes[4];
	unsigned int  match;
	char * name;
} exec_def_t;

exec_def_t fmts[] = {
	{exec_elf, {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3}, 4, "ELF"},
	{exec_shebang, {'#', '!', 0, 0}, 2, "#!"},
};

static int matches(unsigned char * a, unsigned char * b, unsigned int len) {
	for (unsigned int i = 0; i < len; ++i) {
		if (a[i] != b[i]) return 0;
	}
	return 1;
}

/**
 * Load an execute a binary.
 *
 * This determines the binary type (eg., ELF binary, she-bang script, etc.)
 * and then calls the appropriate underlying exec function.
 *
 * @param path Path to the executable to attempt to execute.
 * @param argc Number of arguments (because I'm not counting for you)
 * @param argv Pointer to a string of arguments
 */
extern fs_node_t * fnodeGLOBAL;
int exec(
		char *  path, /* Path to the executable to run */
		int     argc, /* Argument count (ie, /bin/echo hello world = 3) */
		char ** argv, /* Argument strings (including executable path) */
		char ** env   /* Environmen variables */
	) {
	/* Open the file */
	fs_node_t * file = kopen2(path,0);
	if (!file) {
		/* Command not found */
		return -ENOENT;
	}

	/* Read four bytes of the file */
	unsigned char head[4];
	read_fs(file, 0, 4, head);
 
	debug_print(WARNING, "First four bytes: %c%c%c%c", head[0], head[1], head[2], head[3]);

	new_task->name = strdup(path);
	gettimeofday((struct timeval *)&new_task->start, NULL);

	for (unsigned int i = 0; i < sizeof(fmts) / sizeof(exec_def_t); ++i) {
		if (matches(fmts[i].bytes, head, fmts[i].match)) {
			debug_print(WARNING, "Matched executor: %s", fmts[i].name);
			return fmts[i].func(path, file, argc, argv, env, 0);
		}
	}

	debug_print(WARNING, "Exec failed?");
	return -ENOEXEC;
}


int
system(
		char *  path, /* Path to the executable to run */
		int     argc, /* Argument count (ie, /bin/echo hello world = 3) */
		char ** argv  /* Argument strings (including executable path) */
	) {
	char ** argv_ = valloc(sizeof(char *) * (argc + 1));
	for (int j = 0; j < argc; ++j) {
		argv_[j] =  valloc((strlen(argv[j]) + 1) * sizeof(char));
		memcpy(argv_[j], argv[j], strlen(argv[j]) + 1);
	}
new_task = valloc(sizeof(task_t)); 
	argv_[argc] = 0;
	char * env[] = {NULL};
	set_process_environment((task_t*)new_task, clone_directory(current_directory));
	current_directory = new_task->thread.page_directory;
	switch_page_directory(current_directory);

	new_task->cmdline = argv_;

 	//load_elf(NULL,NULL,NULL,NULL,NULL,NULL);
	  exec(path,argc,argv_,env);

	//debug_print(ERROR, "Failed to execute process!");
	// exit(-1);
	return -1;
}





