/* vim: tabstop=4 shiftwidth=4 noexpandtab
 */
#pragma once

#include <system.h>
#include <types.h>

#define SHM_PATH_SEPARATOR "."

/* Types */
struct shm_node;

typedef struct {
	struct shm_node * parent;
	volatile unsigned char lock;
	unsigned int ref_count;

	uint32_t num_frames;
	uintptr_t *frames;
} shm_chunk_t;

typedef struct shm_node {
	char name[256];
	shm_chunk_t * chunk;
} shm_node_t;

typedef struct {
	shm_chunk_t * chunk;
	unsigned char volatile lock;

	unsigned int num_vaddrs;
	uintptr_t *vaddrs;
} shm_mapping_t;

/* Syscalls */
extern void * shm_obtain(char * path, size_t * size);
extern int    shm_release(char * path);

/* Other exposed functions */
extern void shm_install(void);
//extern void shm_release_all(struct task * proc);


