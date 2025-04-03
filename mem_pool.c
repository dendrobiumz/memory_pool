#include "mem_pool.h"
#include <sys/mman.h>
#include <stdlib.h>


typedef struct free_mem {
    struct node node;
    int         idx;
} free_mem_t;



static void* alloc(size_t num)
{

}


static int free(void* addr)
{
    return 0;
}


mem_pool_t* new_mem_pool(int obj_sz, int obj_nbrs)
{
    mem_pool_t *mpl = (mem_pool_t*) malloc(sizeof(mem_pool_t));
    if (mpl == NULL)
        return NULL;

    mpl->blk_nbr = obj_nbrs;
    mpl->blk_sz = obj_sz;
    
    mpl->mem = mmap(NULL, obj_sz * obj_nbrs, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mpl->mem == MAP_FAILED) {
        free(mpl);
        return NULL;
    }
    free_mem_t *f = (free_mem_t*) mpl->mem;
    f->node.next = NULL;
    mpl->list_head.next = &f->node;
    mpl->alloc_mem = alloc;
    mpl->free_mem = free;
    mpl->first_idx = 0;
    return mpl;
}

int destroy_mem_pool(mem_pool_t *mpl)
{
    if (munmap(mpl->mem, mpl->blk_nbr * mpl->blk_sz) == -1)
        return -1;

    free (mpl);
    mpl->mem = NULL;
    mpl->blk_nbr = 0;
    mpl->blk_sz = 0;
    mpl->first_idx = -1;

    return 0;
}


int init_mem_pool(mem_pool_t *mpl, int obj_sz, int obj_nbrs)
{
    mpl->blk_nbr = obj_nbrs;
    mpl->blk_sz = obj_sz;
    
    mpl->mem = mmap(NULL, obj_sz * obj_nbrs, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mpl->mem == MAP_FAILED)
        return -1;
    
    free_mem_t *f = (free_mem_t*) mpl->mem;
    f->node.next = NULL;
    mpl->list_head.next = &f->node;
    mpl->alloc_mem = alloc;
    mpl->free_mem = free;
    mpl->first_idx = 0;
    return 0;
}

int delete_mem_pool(mem_pool_t *mpl)
{
    if (munmap(mpl->mem, mpl->blk_nbr * mpl->blk_sz) == -1)
        return -1;
    
    mpl->blk_nbr = 0;
    mpl->blk_sz = 0;
    mpl->first_idx = -1;
    mpl->mem = NULL;
    return 0;
}