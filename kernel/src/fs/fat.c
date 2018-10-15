#if 0
#include <fat.h>
#include <video.h>
#include <types.h>
#include <heapmngr.h>
#include <kutils.h>
#include <ata.h>
#include <elf.h>
#include <proc.h>
#include <fat.h>
#include <vesa.h>
#include <vmmngr.h>
#define NUM_FILES 8
#define NUM_entries 16
BOOTSECTOR_t *bootsector;
unsigned int* SCREEN_;
u32 FAT[2000];
MOUNT_INFO _MountInfo;
u32 first_free_cluster();
u16 gcluster; 

elf_header_t * read_elf(FILE file);
int fd =0;

FILE ls_dir( char* DirectoryName, u32 offset);
FILE parse_dir( char* DirectoryName);
u32 scan_free_entry();
char file_meta_data[] =
{
 0x41, 0x6d, 0x0, 0x75, 0x00, 0x75, 0x00, 0x2e,  0x00, 0x74, 0x00, 0x0f, 0x00, 0xaf, 0x78, 0x00,  
 0x74, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
 0x4d, 0x55, 0x55, 0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x58, 0x54, 0x20, 0x00, 0x00, 0x39, 0x0a,  
 0x45, 0x43, 0x45, 0x43, 0x00, 0x00, 0x39, 0x0a,  0x45, 0x43, 0x03, 0x00, 0x1b, 0x00, 0x00, 0x00

};

void mount_fat32()
{
	set_term_color(make_color(COLOR_BLACK,COLOR_WHITE));
	bootsector = (BOOTSECTOR_t*)malloc_(sizeof(BOOTSECTOR_t));
	read_disc_sector(0,bootsector,0);
	read_disc_sector(bootsector->sectors_per_fat,FAT,bootsector->reserved_sectors);
	_MountInfo.numSectors = bootsector->total_sectors;
	_MountInfo.rootOffset = (bootsector->num_fats * bootsector->sectors_per_fat) + bootsector->reserved_sectors;
	
	printk("Sectors available: %x\n", _MountInfo.numSectors );
	printk("rootOffset: %d\n", _MountInfo.rootOffset);
	printk("Test: %x\n", (u32)&bootsector->total_sectors - (u32)bootsector);
	printk("root_cluster_num: %d\n", bootsector->root_cluster_num);
	printk("num_direntries: %d\n",bootsector->num_direntries);
	printk("sector per cluster %d\n", bootsector->sectors_per_cluster);
	printk("sectors_per_fat %d\n", bootsector->num_fats);
	
	//FAT = (u32*)malloc_(bootsector->sectors_per_fat * 512); 
	
	
	free(bootsector);
	
}
u16 *surface;

#define VESA_MODE 324
void FAT_shell_newlib()
{
	FILE elf;
	elf_header_t * elf_program;
	elf = ls_dir("SHELL",0);
	elf_program = read_elf(elf);
	create_process((void*)elf_program->entry,THREAD,3,0,0);
	for(;;);
}

int FAT_vesa()
{
	elf_header_t * elf_program;
	elf_header_t * elf_program2;
	int file = 0;
	int sz = 0;
	FILE elf;
	u16 gmode = 324;
	
	elf = ls_dir("VESA",0);
	
	elf_program = read_elf(elf);
			
	*(u16*)0x3600 = gmode;
	
    
	 create_task_vm86((void*)elf_program->entry,"vm86");
	 
	
	sleep2(400);
	memcpy(&mib, (void*)0x3600, sizeof(VESA_MODE_INFO));
    
	*(u16*)0x3600 = 0xC1FF&(0xC000|gmode);
	
	
    printk("PhysBasePtr: %x\n",mib.PhysBasePtr);
	
	printk("XResolution: %d pixels \n",mib.XResolution);
	printk("XResolution: %d pixels \n",mib.YResolution);
	printk("BitsPerPixel: %d\n", mib.BitsPerPixel);
	printk("WinSize %d\n", mib.WinSize);

	sleep2(100);
	elf = ls_dir("GLOADER",0);
	
	elf_program = read_elf(elf);
	printk("test");
	sleep2(200);
	// u32 numberOfPages = 0x1000;
	 u32 numberOfPages = 0x1000; //0x2000 for other
     SCREEN_ = map_to_physical(mib.PhysBasePtr, numberOfPages);
	printk("%x\n", SCREEN_);
//memset(0xa0000000,255,1024*768*4);
	
	 create_task_vm86((void*)elf_program->entry,"vm86");
	 /* int i = 0;
			  while(i < 1000)
			  {
					memset(0xa0000000,i,1024*768*4);
					i++;
			  } */
	
//	printk("do you see this11?\n");
	//sleep(2000);
	//memset(0xa0000000,1, mib.XResolution*mib.YResolution*4);	/* put whole screen painted! */
	//sleep(2000);
	//__asm__ __volatile__("sti");
	//memset(0xa0000000,130, 1024*768*4);
	return 1;
	//memset(SCREEN,50, mib.XResolution*mib.YResolution*4);

}
unsigned int * giveSCREEN_()
{
	printk("SCREEN = %x\n", SCREEN_);
	return SCREEN_;
}
void FAT_file_testing()
{
	/*int ret = scan_free_entry();
	printk("%d", ret);*/
	
	int file;		
	int next;
	FILE test; 
	int i = 0;
	char buf[4096]; /*important*/
	test = ls_dir("vesa",0);
	file = open("text.txt",0);
	read(file,buf,current_task->fdtable[file].size);
	while(i < current_task->fdtable[file].size)
	putch(buf[i++]);
	/*	
	char *name = "kosmisk";
	next = open(name,1);		
	write(next,"HEY how are you doin?!", strlen(name)); 
	next = open("kosmisk.TXT",0);	
    read(next,0,current_task->fdtable[next].size);*/
}

#define FS_FILE       0
#define FS_DIRECTORY  1
#define FS_INVALID    2
#define ATTRIB_READONLY 0x1
#define ATTRIB_HIDDEN 0x2
#define ATTRIB_SYSTEM 0x4
#define ATTRIB_VOLUME_ID 0x8
#define ATTRIB_DIR 0x10
#define ATTRIB_ARCHIVE 0x20

/* Used for Long File Name entries */
#define ATTRIB_LFN (ATTRIB_READONLY | ATTRIB_HIDDEN | ATTRIB_SYSTEM | ATTRIB_VOLUME_ID)

#define SECTOR_PER_CLUSTER 8
#define CLUSTER_SIZE 4900000
#define SECTOR_SIZE 512
#define FIRST_FAT_SECTOR 1

 u32 fat_next_cluster(u32 cur_cluster) { 
	 if(FAT[cur_cluster] >= 0x0FFFFFF8)
	 return 0x0FFFFFFF; // EOC

	
	u32 val; 
	    val = FAT[cur_cluster]; 
   return (val & 0x0FFFFFFF);
	      }
u8 FAT_table[CLUSTER_SIZE];

elf_header_t * elf_header_;

elf_header_t * read_elf(FILE file )
{
if(file.fileLength > 1000000)
{	
		#define EOC 0x0FFFFFFF
	//file_buf = (u32*)malloc_a(334689 );
	//memset(file_buf,0,334689);
	 u32 bytes_read = 0;
	u32 bytes_left;
	u32 current_cluster; //= file.currentCluster;
	int i = 0;
	u32 start_lba = 0;
	  	memset(FAT_table, 0, 4900000);
		  current_cluster = file.currentCluster;
	for (bytes_left = (u32)file.fileLength; bytes_left > 0; bytes_left -= 4096) {  

	
	if (current_cluster > 2 && current_cluster < 0x0ffffff7) { 
	//	printk("reading cluster %d to %d, bytes_left = %d, bytes_read = %d\n", current_cluster, FAT_table + bytes_read, bytes_left, bytes_read);
	    start_lba = _MountInfo.rootOffset + (current_cluster - 2) * SECTOR_PER_CLUSTER;
		read_disc_sector(10,FAT_table + bytes_read,start_lba); 	
	bytes_read +=  4096;
		
	}
	current_cluster = fat_next_cluster(current_cluster);
		
		if(current_cluster == EOC)
		 break;	
i++;

}
sleep2(1000);
	elf_header_ = elf_exec(FAT_table,  file.fileLength, file.name,0,0);
               

	return elf_header_; 
	
}
else 
{	
	

	
//	memset(FAT_table, 0, 4900000);
	u32 sector_count = file.fileLength/SECTOR_SIZE;
	printk("sector count : %d\n", sector_count);
	u32 cluster_start_lba = _MountInfo.rootOffset+(file.currentCluster - 2) * SECTOR_PER_CLUSTER;
	printk("cluster_start_lba %d\n", cluster_start_lba);
	read_disc_sector(sector_count,FAT_table,cluster_start_lba);
sleep(1000);
	//elf_header_ = parse_elf(FAT_table, file.fileLength);
	elf_header_ = elf_exec(FAT_table,  file.fileLength, file.name,0,0);
               

	return elf_header_;
}   
/*
int i;
for(i = 0; i < file.fileLength; i++)
	{	
		if(!file.eof)
			putch(FAT_table[i]);
	}*/
}

int read_(FILE file )
{
	u32 sector_count = 1 ; /*file.fileLength/SECTOR_SIZE;*/
	u32 cluster_start_lba = _MountInfo.rootOffset+(gcluster - 2) * SECTOR_PER_CLUSTER;
	read_disc_sector(sector_count,FAT_table,cluster_start_lba);
	return file.fileLength;
}

int read_file(FILE file, char *buffer )
{
	int ret;
	/*	printk("file size : %d\n", current_task->fdtable[fd].size);
		*/
	u32 sector_count =current_task->fdtable[fd].size/SECTOR_SIZE;
	/*printk("sector count: %d\n", sector_count);*/
	u32 cluster_start_lba = _MountInfo.rootOffset+(current_task->fdtable[fd].node_[current_task->fdtable[fd].size].currentCluster - 2) * SECTOR_PER_CLUSTER;
	read_disc_sector(sector_count,buffer,cluster_start_lba);
ret++;
	/*int i;
	for(i = 0; i < current_task->fdtable[fd].size; i++)
	{	
	
			putch(buffer[i]);
		
	}
	printk("\n");*/
	return ret;
}
#define LBAoffset 5

void write_file(FILE file , char *buf, u8 method, u32 offset)
{	
	
	
	u32 cluster_start_lba;
	if(method == 1) /*write metadata e.g create files*/
	{
		cluster_start_lba = _MountInfo.rootOffset+1;	
		read_(file);
		memcpy(FAT_table+ 0, buf, 32);
		FAT_table[cluster_start_lba] = 0x0FFFFFFF;
		write_disc_sector(cluster_start_lba,FAT_table);
	}	

	if(method == 2) /*write contents to that file*/
	{
		cluster_start_lba =  _MountInfo.rootOffset+(gcluster - 2) * SECTOR_PER_CLUSTER;
		read_(file);
		memcpy(FAT_table+0, buf, 32);
		FAT_table[cluster_start_lba] = 0x0FFFFFFF;
		write_disc_sector(cluster_start_lba,FAT_table);
	}	
}


#define NUM_FILES 20



FILE ls_dir( char* DirectoryName, u32 offset)
{	
	DIRECTORY *lsdirectory;
	FILE file;
	char DosFileName[11];
	ToDosFileName (DirectoryName, DosFileName, 11);
	DosFileName[11]=0;
	unsigned char buf[512];
	lsdirectory = (DIRECTORY*)malloc_(32);	
	u32 backupinfo = _MountInfo.rootOffset;
	memset(lsdirectory, 0, 32);
	read_disc_sector(0,buf,_MountInfo.rootOffset+offset);
	
int counter = 0;
	int i;
	while(counter < 128)
	{
	lsdirectory = (DIRECTORY*) buf;

	    for (i=0; i<NUM_FILES; i++) {		
    			char name[11];
    		
    			memcpy (name, lsdirectory->Filename, 11);
    				
    			name[11]=0;
    				if (lsdirectory->Attrib == 0x20 || lsdirectory->Attrib == 0x10)
    				{
					if (strncmp (DosFileName, lsdirectory->Filename,11) == 0) {
				
					printk(" Filename: %s\n", name);
					printk(" FileSize: %d BYTES\n", lsdirectory->FileSize);
					printk(" Attrib: %d\n", lsdirectory->Attrib);
					printk(" FirstCluster %d\n", lsdirectory->FirstCluster);
					
					file.Attrib  		= lsdirectory->Attrib;
					memcpy (file.name, lsdirectory->Filename, 8);
					memcpy (file.Ext, lsdirectory->Ext, 3);
					file.id             = 0;
					file.currentCluster = lsdirectory->FirstCluster;
					file.eof            = 0;
					file.fileLength     = lsdirectory->FileSize;
					
					if (lsdirectory->Attrib == 0x10)
					file.flags = FS_DIRECTORY;
					else
					file.flags = FS_FILE;
					_MountInfo.rootOffset = backupinfo;
					/*free(directory);*/
					return file;
						/*return directory->Filename;*/
					}
				}
    	 lsdirectory++; 
    	counter++;
   	 
	}
	if(counter > 128)
		read_disc_sector(0,buf,_MountInfo.rootOffset+offset);
  }
	
	/*SUBdirs and subfiles and prints out subfiles data*/
/*	directory = (DIRECTORY*) &FAT_table;

	    for (i=0; i<NUM_FILES; i++) {		
    			char name[11];
    			memcpy (name, directory->Filename, 11);
    			name[11]=0;
    				if (directory->Attrib == 0x20)
    				{
						printk("%s %d BYTES\n",name, directory->FileSize);
						file.id             = 0;
						file.currentCluster = directory->FirstCluster;
						file.eof            = 0;
						file.fileLength     = directory->FileSize;
					    read_file(file);					    
					}
    	 directory++;
	}*/
_MountInfo.rootOffset = backupinfo;
	return file;
}

u32 first_free_cluster() { 
	int i;
	for ( i=0; i <  _MountInfo.numSectors; i++) {
		if (FAT_table[i*512] == 0) return i;
	}
}

u32 first_free_entry() { 
	int i;
	for ( i=0; i <  _MountInfo.numSectors; i++) {
		if (FAT_table[i*32] == 0) return i*32;
	}
}
#define O_RDONLY 0
#define O_WRONLY 1537


int open(const char *path, int mode) {
	fd++;
	printk("\n");
	printk("Mode: %d\n", mode);
	DIRECTORY *mdirectory;
	

	mdirectory = (DIRECTORY*)malloc_(32);
	memset(mdirectory, 0, 32);	

	if ( mode == O_WRONLY)
	{	
		printk("Creating a file...\n");
		FILE enode;
		memcpy (mdirectory->Filename, path, 8);
		memcpy (mdirectory->Ext, "txt", 3);
		printk("Filename -> %s\n",mdirectory->Filename );
		mdirectory->Attrib = 0x20;
		mdirectory->Reserved = 0x00;
		mdirectory->TimeCreatedMs = 0x00;
		mdirectory->TimeCreated = 0x0a39;
		mdirectory->DateCreated = 0x4345;
		mdirectory->DateLastAccessed = 0x4345; 
		mdirectory->LastModTime = 0x0000;
		mdirectory->LastModDate = 0x0a39;

		/* important */
		mdirectory->FirstCluster = first_free_cluster(); 
		gcluster = mdirectory->FirstCluster;
		mdirectory->FileSize = 0x0000001b;  
		enode.currentCluster = 1; 
		enode.fileLength = 0x0000001b;
	
		char buffer[32];
		
		memcpy(buffer, mdirectory, 32); 
		write_file(enode,buffer,1,0); 
		
		current_task->fdtable[fd].node_[5] = enode; /* node[x] -> x = size of write message , todo: fix this !*/
		return fd;		
	}
	if(mode == O_RDONLY)
	{	 		
		FILE node;
		node = ls_dir(path,0);
		printk(" FD num: %d\n", fd);
		current_task->fdtable[fd].size = node.fileLength;
		current_task->fdtable[fd].node_[node.fileLength] = node;
		printk("\n");
		return fd;
		/* read file */
	}
}




int x = 0;
int read(int file,  void *buffer, u32 size)
{
	int ret = 0;
	if(file == 0)
	{
	ret = stdio_read(file,buffer,size);
	return ret;
}
	else
	{
	if(x > 0)
	{
		x = 0;
		return -1;
	}
	ret = read_file(current_task->fdtable[file].node_[size],buffer);
	x++;
	return current_task->fdtable[fd].size;
}
	
return ret;
}

int write(int file, char* buf, int length)
{
	/*printk("write file is %d\n", file);*/
	int ret = 0;
	if(file > 1)
	{
		write_file(current_task->fdtable[fd].node_[length],buf,2,0);
	}
	if(file <= 1)
	{
		const char *p = (const char *)buf;
		int i;
		for (i = 0; i < length && *p; i++) {
			putch(*p++);	
			
	ret++;
	}
}
return ret;
}
#endif
