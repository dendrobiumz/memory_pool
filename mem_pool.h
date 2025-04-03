#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

//#include "lfq.h"
#include <stdatomic.h>
#include <stdint.h>



struct node {
    struct node *next;
};

typedef struct mem_pool{
    size_t blk_sz;
    size_t blk_nbr;
    atomic_int first_idx;
    struct node list_head;
    void *mem;
    void* (*alloc_mem)(size_t num);
    int   (*free_mem)(void* addr);    
} mem_pool_t;

mem_pool_t* new_mem_pool(int obj_sz, int obj_nbrs);

int init_mem_pool(mem_pool_t *mpl, int obj_sz, int obj_nbrs);

#endif