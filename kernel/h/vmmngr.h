#ifndef PAGING_H
#define PAGING_H
#include <types.h>
#define PAGESIZE            0x1000  // size
 

typedef struct page {
	unsigned int present:1;
	unsigned int rw:1;
	unsigned int user:1;
	unsigned int writethrough:1;
	unsigned int cachedisable:1;
	unsigned int unused:7;
	unsigned int frame:20;
} __attribute__((packed)) page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory {
	uintptr_t physical_tables[1024];	/* Physical addresses of the tables */
	page_table_t *tables[1024];	/* 1024 pointers to page tables... */
	uintptr_t physical_address;	/* The physical address of physical_tables */
	unsigned int ref_count;
} page_directory_t;


 void _vmmngr_alloc_frame(page_t *page, int is_kernel, int is_writeable);
page_t *_vmm_get_page_addr(u32 addr, u32 make, page_directory_t  *dir);
u32 p_kmalloc(u32 size, u32 align, u32 *phys);


u32 paging_getPhysAddr(void* virtAddress);
void* paging_getVirtaddr(u32 physAddress, u32 numPages);


extern page_directory_t *kernel_directory;

#endif
