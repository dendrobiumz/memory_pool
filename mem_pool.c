#include "mem_pool.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

typedef struct free_mem {
    uint64_t    guard;
    int         next_free_idx;
} free_mem_t;



static void* alloc_mem_block(struct mem_pool *self)
{
    uintptr_t addr = 0;
    int first_free_idx;
    free_mem_t *f;
    // key: first free block index (self->first_free_idx)
    if (self->init_blk_nbrs > self->blk_nbrs)
        return NULL;

    do {
        first_free_idx = atomic_load(&self->first_free_idx);
        addr = self->mem_start + first_free_idx * self->blk_sz;
        if (addr > self->mem_end)
            return NULL;
        f = (free_mem_t*) addr;
    } while (!atomic_compare_exchange_strong(&self->first_free_idx, &first_free_idx, f->next_free_idx));

    atomic_fetch_add(&self->init_blk_nbrs, 1);
    return (void*) addr;
}


static int free_mem_block(struct mem_pool *self, void* addr)
{
    // check addr is valid, in our memory range
    if ((uintptr_t)addr > self->mem_end || (uintptr_t)addr < self->mem_start)
        return -1;

    atomic_int first_free_idx;
    int idx = ((uintptr_t) addr - self->mem_start) / self->blk_sz;
    free_mem_t *f = (free_mem_t*) addr;
    do {
        first_free_idx = self->first_free_idx;
        f->next_free_idx = first_free_idx;
    } while (!atomic_compare_exchange_strong(&self->first_free_idx, &first_free_idx, idx));
   
    return 0;
}


mem_pool_t* new_mem_pool(int obj_sz, int obj_nbrs)
{
    if (obj_sz <= 4)
        return NULL;

    struct mem_pool *mpl = (struct mem_pool*) malloc(sizeof(struct mem_pool));
    if (mpl == NULL)
        return NULL;

    mpl->blk_nbrs = obj_nbrs;
    mpl->blk_sz = obj_sz;
    
    void *mem = mmap(NULL, obj_sz * obj_nbrs, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        free(mpl);
        return NULL;
    }
    memset(mem, 7, obj_sz * obj_nbrs);
    mpl->mem_start = (uintptr_t) mem;
    mpl->mem_end = ((uintptr_t) mem) + obj_sz * obj_nbrs;
    free_mem_t *f = (free_mem_t*)mem;
    f->next_free_idx = 0;

    mpl->first_free_idx = 0;
    mpl->init_blk_nbrs = 0;

    mpl->alloc = alloc_mem_block;
    mpl->free = free_mem_block;
    return mpl;
}

int destroy_mem_pool(mem_pool_t *mpl)
{
    if (munmap((void*) (mpl->mem_start), mpl->blk_nbrs * mpl->blk_sz) == -1)
        return -1;

    free (mpl);
    return 0;
}


int init_mem_pool(mem_pool_t *mpl, int obj_sz, int obj_nbrs)
{
    if (obj_sz <= 4)
        return -1;
    mpl->blk_nbrs = obj_nbrs;
    mpl->blk_sz = obj_sz;
    
    void *mem = mmap(NULL, obj_sz * obj_nbrs, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
        return -2;

    memset(mem, 7, obj_sz * obj_nbrs);
    mpl->mem_start = (uintptr_t) mem;
    mpl->mem_end = ((uintptr_t) mem) + obj_sz * obj_nbrs;
    free_mem_t *f = (free_mem_t*) mem;
    f->next_free_idx = 0;

    mpl->alloc = alloc_mem_block;
    mpl->free = free_mem_block;
    mpl->first_free_idx = 0;
    mpl->init_blk_nbrs = 0;
    return 0;
}

int delete_mem_pool(mem_pool_t *mpl)
{
    if (munmap((void*) (mpl->mem_start), mpl->blk_nbrs * mpl->blk_sz) == -1)
        return -1;
    
    mpl->blk_nbrs = 0;
    mpl->blk_sz = 0;
    mpl->first_free_idx = -1;
    mpl->init_blk_nbrs = 0;

    return 0;
}