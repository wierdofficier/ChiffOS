#ifndef PAGING_H
#define PAGING_H
#include <types.h>
void _vmmngr_initialize(struct multiboot *mbp);

struct p_pages
{
   u32 present    : 1;   
   u32 rw         : 1;   
   u32 user       : 1;  
   u32 accessed   : 1;   
   u32 dirty      : 1;   
   u32 unused     : 7;   
   u32 frame_addr      : 20;
}__attribute__((packed));

struct p_tables
{
	struct p_pages pages[1024];
}__attribute__((packed));

struct p_directory
{
	struct p_tables *tables[1024];
	u32 tablephysical[1024];
}__attribute__((packed));



#endif
