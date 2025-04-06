#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include <stdatomic.h>
#include <stdint.h>


typedef struct mem_pool{
    int blk_sz;
    int blk_nbrs;
    atomic_int first_free_idx;
    atomic_int init_blk_nbrs;
    uintptr_t mem_start;
    uintptr_t mem_end;
    void* (*alloc)(struct mem_pool *self);
    int   (*free)(struct mem_pool *self, void* addr);    
} mem_pool_t;

mem_pool_t* new_mem_pool(int obj_sz, int obj_nbrs);

int init_mem_pool(mem_pool_t *mpl, int obj_sz, int obj_nbrs);

#endif