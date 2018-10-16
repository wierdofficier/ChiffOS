#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include <types.h>

#define MULTIBOOT_MAGIC        0x1BADB002
#define MULTIBOOT_EAX_MAGIC    0x2BADB002
#define MULTIBOOT_FLAG_MEM     0x001
#define MULTIBOOT_FLAG_DEVICE  0x002
#define MULTIBOOT_FLAG_CMDLINE 0x004
#define MULTIBOOT_FLAG_MODS    0x008
#define MULTIBOOT_FLAG_AOUT    0x010
#define MULTIBOOT_FLAG_ELF     0x020
#define MULTIBOOT_FLAG_MMAP    0x040
#define MULTIBOOT_FLAG_DRIVE   0x080
#define MULTIBOOT_FLAG_CONFIG  0x100
#define MULTIBOOT_FLAG_LOADER  0x200
#define MULTIBOOT_FLAG_APM     0x400
#define MULTIBOOT_FLAG_VBE     0x800
#define MULTIBOOT_FLAG_FB 0x1000
struct multiboot
{
	unsigned int flags;
	unsigned int mem_lower;
	unsigned int mem_upper;
	unsigned int boot_device;
	unsigned int cmdline;
	unsigned int mods_count;
	unsigned int mods_addr;
	unsigned int num;
	unsigned int size;
	unsigned int addr;
	unsigned int shndx;
	unsigned int mmap_length;
	unsigned int mmap_addr;
	unsigned int drives_length;
	unsigned int drives_addr;
	unsigned int config_table;
	unsigned int boot_loader_name;
	unsigned int apm_table;
	unsigned int vbe_control_info;
	unsigned int vbe_mode_info;
	unsigned int vbe_mode;
	unsigned int vbe_interface_seg;
	unsigned int vbe_interface_off;
	unsigned int vbe_interface_len;
	unsigned int framebuffer_addr;
	unsigned int framebuffer_pitch;
	unsigned int framebuffer_width;
	unsigned int framebuffer_height;
	unsigned char   framebuffer_bpp;
	unsigned char   framebuffer_type;
	/* Palette stuff goes here but we don't use it */
} __attribute__ ((packed));
typedef struct {
	unsigned short  attributes;
	unsigned char   winA, winB;
	unsigned short  granularity;
	unsigned short  winsize;
	unsigned short  segmentA, segmentB;
	unsigned int realFctPtr;
	unsigned short  pitch;

	unsigned short  Xres, Yres;
	unsigned char   Wchar, Ychar, planes, bpp, banks;
	unsigned char   memory_model, bank_size, image_pages;
	unsigned char   reserved0;

	unsigned char   red_mask, red_position;
	unsigned char   green_mask, green_position;
	unsigned char   blue_mask, blue_position;
	unsigned char   rsv_mask, rsv_position;
	unsigned char   directcolor_attributes;

	unsigned int physbase;
	unsigned int reserved1;
	unsigned short  reserved2;
} __attribute__ ((packed)) vbe_info_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef struct {
	unsigned int size;
	uint64_t base_addr;
	uint64_t length;
	unsigned int type;
} __attribute__ ((packed)) mboot_memmap_t;
#endif
