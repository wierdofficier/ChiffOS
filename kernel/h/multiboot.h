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
   u32 flags;
   u32 mem_lower;
   u32 mem_upper;
   u32 boot_device;
   u32 cmdline;
   u32 mods_count;
   u32 mods_addr;
   u32 num;
   u32 size;
   u32 addr;
   u32 shndx;
   u32 mmap_length;
   u32 mmap_addr;
   u32 drives_length;
   u32 drives_addr;
   u32 config_table;
   u32 boot_loader_name;
   u32 apm_table;
   u32 vbe_control_info;
   u32 vbe_mode_info;
   u32 vbe_mode;
   u32 vbe_interface_seg;
   u32 vbe_interface_off;
   u32 vbe_interface_len;
}  __attribute__((packed));

typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef struct {
	unsigned int size;
	uint64_t base_addr;
	uint64_t length;
	unsigned int type;
} __attribute__ ((packed)) mboot_memmap_t;
#endif
