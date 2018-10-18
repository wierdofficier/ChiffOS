#include <types.h>
#include <video.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <timer.h>
#include <multiboot.h>
#include <vmmngr.h>
#include <heapmngr.h>
#include <kbd.h>
#include <fat.h>
#include <proc.h>
#include <lwip/init.h>
#include <lwip/sys.h>
#include <lwip/stats.h>
#include <lwip/ip_addr.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>
#include <lwip/netifapi.h>
#include <lwip/ip_addr.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#include <lwip/stats.h>
#include <netif/etharp.h>
int start_graphics_daemon();
int init_graphics(void);
#define MULTIBOOT_FLAG_MEM 0x001
u32 tmp;
u32 tmp2;
extern void *end;
uintptr_t last_mod = (uintptr_t)&end;
unsigned char *videoram = (unsigned char *)0xb8000;
 
extern uintptr_t placement_pointer;
 
static unsigned long int next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}
typedef unsigned int uint32;
void *max_alloc = NULL;
 #define assert(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
#define IS_DWORD_ALIGNED(x) (( ((uint32)(x)) & 3) == 0)
#define RAND_RANGE(x,y) ( rand() % (y - x + 1) + x )
void heaptest( ) {
	/**********************************
	 *** HEAP DEBUGGING AND TESTING ***
	 **********************************/
#define NUM 10 
#define TEST_1_LOOPS 11
#define TEST_2_LOOPS 10

	unsigned int start_time = gettickcount();
 
	//print_heap_index();

	void *a = valloc(2228);
	void *b = valloc(8222);
	void *c = valloc(83333);

 

	memset(a, 0xab, 6);
	memcpy(b, a, 6);

	printk("a: %x\n", a);
	printk(", b: %x\n", b);
	printk("c: %x\n", c);

 
	//print_heap_index();

	printk("Freeing c...\n");
	free(c);
	//print_heap_index();
	printk("Freeing a...\n");
	free(a);
	//print_heap_index();
	printk("Freeing b...\n");
free(b);
void **p =  valloc(sizeof(void *) * NUM);
memset(p, 0, sizeof(void *) * NUM);
 uint32 total = 0;
	for (int x=0; x < TEST_1_LOOPS; x++) {
		total = 0;

		for (uint32 i = 0; i < NUM; i++) {
			uint32 sz = RAND_RANGE(4, 65535);
			p[i] = valloc(sz);
			assert(IS_DWORD_ALIGNED(p[i]));
			if (p[i] > max_alloc) max_alloc = p[i];
			total += sz;
			printk("alloc #%d (%d bytes, data block starts at %p)\n", i, sz, p[i]);

			//validate_heap_index(false);
			//print_heap_index();
		}
		printk("%d allocs done, in total %d bytes (%d kiB)\n", NUM, total, total/1024);

		/* Free one in "the middle" */
		if (NUM > 100) {
			 free((void *)p[100]);
			p[100] = NULL;
		}

	//	validate_heap_index(false);

		for (uint32 i = 0; i < NUM; i++) {
			 free((void *)p[i]);
			printk("just freed block %d \n", i   );
			p[i] = 0;
			//validate_heap_index(false);
			//print_heap_index();
		}
		printk("%d frees done\n", NUM);

}
}

 
 
 

extern unsigned char  * lfb_vid_memory;
extern  unsigned short term_width ;    /* Width of the terminal (in cells) */
extern  unsigned short term_height;
static void tcpip_init_done(void* arg)
{
	sys_sem_t* sem = (sys_sem_t*)arg;

	//debug_print(NOTICE,"LwIP's tcpip thread has task id %d\n", per_core(current_task)->id);

	sys_sem_signal(sem);
}
err_t rtl8139if_init(struct netif* netif);
static struct netif default_netif;
  int init_netifs(void)
{
	ip_addr_t	ipaddr;
	ip_addr_t	netmask;
	ip_addr_t	gw;
	sys_sem_t	sem;
	err_t		err;

	if(sys_sem_new(&sem, 0) != ERR_OK)
		LWIP_ASSERT("Failed to create semaphore", 0);

	tcpip_init(tcpip_init_done, &sem);

	sys_sem_wait(&sem);
	printk( "TCP/IP initialized.\n");
	sys_sem_free(&sem);
 
#ifdef __aarch64__
	//	LOG_ERROR("Unable to add the network interface\n");

		//return -ENODEV;
#else
		/* Clear network address because we use DHCP to get an ip address */
	 	IP_ADDR4(&gw, 0,0,0,0);
	 	IP_ADDR4(&ipaddr, 0,0,0,0);
	 	IP_ADDR4(&netmask, 0,0,0,0);


 	if ((err = netifapi_netif_add(&default_netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw), NULL, rtl8139if_init, ethernet_input)) == ERR_OK)
	 		goto success;

success:
 
		netifapi_netif_set_default(&default_netif);
		netifapi_netif_set_up(&default_netif);

		printk( "Starting DHCPD...\n");
		netifapi_dhcp_start(&default_netif);
 
		int mscnt = 0;
		int ip_counter = 0;
 
		if (!ip_2_ip4(&default_netif.ip_addr)->addr)
			return 0;
#endif
 

	return 0;
}
 
extern int DOTASKSWITCH;
int socketdemo( )
{
    int socket_desc;
    struct sockaddr_in server;
    char *message , server_reply[6000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printk("Could not create socket");
    }
 
    //ip address of www.msn.com (get by doing a ping www.msn.com at terminal)
    server.sin_addr.s_addr = inet_addr("151.177.53.241");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );
 
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
     
    //Send some data
    message = "GET / HTTP/1.1\r\nHost: exscape.org\r\n\r\n";
 
    if( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
        
    //Receive a reply from the server
    if( recv(socket_desc, server_reply , 6000, 0) < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
 
	
    printk("%s\n",server_reply);
     udelay(3);
    return 0;
}
char * boot_arg = NULL;
void serial_install(void);
struct multiboot *mboot_ptr;
void _vesa_initialize();
void kmain(struct multiboot *mbp, u32 magic)
{
	u32 initrd_location = *((u32*)mbp->mods_addr); //get the adress of initrd module
	u32 initrd_end = *(u32*)(mbp->mods_addr+4); 
	placement_pointer = initrd_end; 
	mboot_ptr = mbp;
	init_video_term();
 	puts("Welcome to chiffOS");
 	printk("\n");
 
	videoram[1] = 2;
	videoram[3] = 4;
	videoram[5] = 6;
	videoram[7] = 8;
	videoram[9] = 10;
	videoram[11] = 12;
	videoram[13] = 14;
	videoram[15] = 12;
	videoram[17] = 10;
	videoram[19] = 8;
	videoram[21] = 6;
	videoram[23] = 4;
	videoram[25] = 2;
	videoram[27] = 6;
	videoram[29] = 10;
	videoram[31] = 14;

 
 	printk("Initializing GDTs... ");
 	gdt_install();
 	printk("[ok]\n");
  
 	printk("Initializing IDTs... ");
 	idt_install();
 	printk("[ok]\n");
 
 	printk("Initializing ISRs and enabling interrupts... ");
 	isrs_install();
 	irq_install();
 	printk("[ok]\n");
 
 	printk("\n");
 	printk("Initializing the PIT... ");
 	timer_install(100);
 	_install_syscall_handler();
 
 	printk("[ok]\n");
 	serial_install();
	if ((uintptr_t)mbp > last_mod) {
			last_mod = (uintptr_t)mbp + sizeof(struct multiboot);
	}
	 printk("Initializing paging and the heap... \n");
	while (last_mod & 0x7FF) last_mod++;
		kmalloc_startat(last_mod);

		if (mbp->flags & MULTIBOOT_FLAG_MEM) {
			paging_install(mbp->mem_upper + mbp->mem_lower);
		} else {
			printk(  "Missing MEM flag in multiboot header\n");
		}

		if (mbp->flags & MULTIBOOT_FLAG_MMAP) {
			printk(  "Parsing memory map.\n");
			mboot_memmap_t * mmap = (void *)mbp->mmap_addr;
			while ((uintptr_t)mmap < mbp->mmap_addr + mbp->mmap_length) {
				if (mmap->type == 2) {
					for (unsigned long long int i = 0; i < mmap->length; i += 0x1000) {
						if (mmap->base_addr + i > 0xFFFFFFFF) break; /* xxx */
						//printk(  "Marking 0x%x", (uint32_t)(mmap->base_addr + i));
						paging_mark_system((mmap->base_addr + i) & 0xFFFFF000);
				}	
			}
			mmap = (mboot_memmap_t *) ((uintptr_t)mmap + mmap->size + sizeof(uintptr_t));
		}
	}

	paging_finalize();
	heap_install(); 
 	printk("[ok]\n");
  	//heaptest();
	fpu_install();

 	printk("Setting up the keyboard handler... ");
 	_kbd_initialize();
 	printk("[ok]\n");
	_vesa_initialize();
 	init_graphics();
	memset((void *)(uintptr_t )lfb_vid_memory, 0, term_width * term_height * 12);
 	printk("Initializing initrd... ");
 	//fs_root = install_initrd(initrd_location);
        printk("[ok]\n");
	initialize_process_tree();
	char * cmdline;

	char cmdline_[1024];

	size_t len = strlen((char *)mbp->cmdline);
	memmove(cmdline_, (char *)mbp->cmdline, len + 1);

	/* Relocate the command line */
	cmdline = (char *)kmalloc(len + 1);
	memcpy(cmdline, cmdline_, len + 1);
	
	if (cmdline) {
		args_parse(cmdline);
}


	vfs_install();
	if (args_present("start")) {
		char * c = args_value("start");
		if (!c) {
			debug_print(WARNING, "Expected an argument to kernel option `start`. Ignoring.");
		} else {
			debug_print(NOTICE, "Got start argument: %s", c);
			boot_arg = strdup(c);
		}
}

	printk("Initializing tasking... ");
        _task_initialize();
 	printk("[ok]\n");

	// create_task_thread(start_graphics_daemon,PRIO_HIGH); 
	 // TASK_testing();

 
	// for(;;);
	shm_install();
	map_vfs_directory("/dev");
	
	ata_initialize();

	tmpfs_initialize();
	ext2_initialize();

	char * root_type = "ext2";
	if (args_present("root_type")) {
		root_type = args_value("root_type");
	}
	vfs_mount_type(root_type, args_value("root"), "/");
  

	char * boot_app = "/bitmap";
	if (args_present("bitmap")) {
		boot_app = args_value("bitmap");
	}

	/* Prepare to run /bin/init */
	char * argv[] = {
		boot_app,
		0,
		NULL
	};
	int argc = 0;
	while (argv[argc]) {
		argc++;
	}

 	DOTASKSWITCH=1;
  // TASK_testing();


 init_netifs();
 	    socketdemo();
	 system(argv[0], argc, argv); /* Run init */

  	 //insert_current_task(current_task);

	///
	         
 
for(;;);

}
