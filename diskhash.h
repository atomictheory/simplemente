
#ifndef DISKHASH_H
#define DISKHASH_H
#endif

#ifndef DEEP_H
#include "deep.h"
#endif

#ifndef MEMORY_H
#include "memory.h"
#endif

#define DISK_HASH_SHIFT (13)

#define DISK_POSITION_HASH_SIZE (1 << DISK_HASH_SHIFT)
#define DISK_POSITION_HASH_MASK (DISK_POSITION_HASH_SIZE - 1)

extern void init_disk_hash();

extern void disk_size_info();