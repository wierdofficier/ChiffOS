#if 0
#ifndef FAT_H_
#define FAT_H_
#include <types.h>
#include <elf.h>

struct _BOOT_SECTOR{
u8 jmp[3]; /* x86 code to jump past the following data */
char oem_ident[8];
u16 bytes_per_sector;
u8 sectors_per_cluster;
u16 reserved_sectors;
u8 num_fats;
u16 num_direntries;
u16 small_total_sectors; /* 0 if >65535 sectors, i.e. for all FAT32 partitions? see below */
u8 media_descr_type; /* 0xf8 == hard disk */
u16 ignore; /* FAT12/FAT16 only */
u16 sect_per_track;
u16 heads;
u32 hidden_sectors; /* relative LBA */
u32 total_sectors; /* used if >65535 sectors, i.e. all FAT32 partitions? */

/* Describes the FAT32 EBPB, located just after the BPB (above).
* Note that if this struct is mapped onto a partition that is actually
* FAT12/FAT16, the values will be wildly incorrect!
*/
u32 sectors_per_fat; /* FAT size, in sectors */
u16 flags;
u8 fat_major_version;
u8 fat_minor_version; /* these two MAY be swapped */
u32 root_cluster_num;
u16 fsinfo_cluster_num;
u16 backup_bs_cluster_num;
u8 reserved[12]; /* should be all 0 */
u8 drive_number;
u8 reserved2; /* "Flags in Windows NT. Reserved otherwise. */
u8 signature; /* 0x28 or 0x29 */
u32 volumeid_serial;
char volume_label[11]; /* space padded */
} __attribute__((packed));

typedef struct _DIRECTORY {

	char   Filename[8];           
	u8   Ext[3];                
	u8   Attrib;                
	u8   Reserved;
	u8   TimeCreatedMs;         
	u16  TimeCreated;
	u16  DateCreated;           
	u16  DateLastAccessed;
	u16  FirstClusterHiBytes;
	u16  LastModTime;           
	u16  LastModDate;
	u16  FirstCluster;         
	u32  FileSize;            

}__attribute__((packed));


typedef struct _DIRECTORY DIRECTORY;
typedef struct _BOOT_SECTOR BOOTSECTOR_t;
typedef struct _MOUNT_INFO {

	u32 numSectors;
	u32 fatOffset;
	u32 numRootEntries;
	u32 rootOffset;
	u32 rootSize;
	u32 fatSize;
	u32 fatEntrySize;

}MOUNT_INFO;

typedef struct _FILE {

	char   name[8];
	u8 	   Ext[3];
	u8     Attrib;
	u32    flags;
	u32    fileLength;
	u32    id;
	u32    eof;
	u32    position;
	u32    currentCluster;
	u32    device;

}__attribute__((packed));
typedef struct _FILE FILE;

void mount_fat32();
int FAT_vesa();
void write_file(FILE file , char *buf, u8 method, u32 offset);
int open(const char *path, int mode);
elf_header_t * read_elf(FILE file );
int write(int file, char* buf, int length);
void FAT_shell_newlib();
FILE ls_dir( char* DirectoryName, u32 offset);
int read(int file,  void *buffer, u32 size);
unsigned int * giveSCREEN();
#endif
#endif
