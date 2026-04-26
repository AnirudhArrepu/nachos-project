#include "copyright.h"
#include "debug.h"
#include "kernel.h"
#include "addrspace.h"
#include "malloc.h"
extern void handle_PageFault();
extern Kernel *kernel;

static unsigned int free_list_head  = 0;
static unsigned int alloc_list_head = 0;
static bool         initialized     = false;

static char *user_to_kernel(AddrSpace *space, unsigned int vaddr)
{
    unsigned int vpn    = vaddr / PageSize;
    unsigned int offset = vaddr % PageSize;
    ASSERT(vpn < (unsigned int)space->numPages);

    if(!space->pageTable[vpn].valid){
	kernel->machine->WriteRegister(BadVAddrReg, vaddr);
	handle_PageFault();
    }

    ASSERT(space->pageTable[vpn].valid);
    int ppn = space->pageTable[vpn].physicalPage;
    ASSERT(ppn != -1);
    return &(kernel->machine->mainMemory[ppn * PageSize + offset]);
}

static blk_metadata *meta(AddrSpace *space, unsigned int vaddr)
{
    return (blk_metadata *)user_to_kernel(space, vaddr);
}

static void fl_add(unsigned int blk_vaddr)
{
    AddrSpace    *space = kernel->currentThread->space;
    blk_metadata *blk   = meta(space, blk_vaddr);
    blk->is_free   = true;
    blk->n_blk     = free_list_head;
    free_list_head = blk_vaddr;
}

static void fl_remove(unsigned int blk_vaddr)
{
    AddrSpace    *space = kernel->currentThread->space;
    blk_metadata *blk;
    unsigned int  prev_vaddr;
    blk_metadata *prev;

    if (free_list_head == 0) return;

    if (free_list_head == blk_vaddr) {
        blk = meta(space, free_list_head);
        free_list_head = blk->n_blk;
        return;
    }

    prev_vaddr = free_list_head;
    prev       = meta(space, prev_vaddr);

    while (prev->n_blk != 0) {
        if (prev->n_blk == blk_vaddr) {
            blk_metadata *target = meta(space, blk_vaddr);
            prev->n_blk = target->n_blk;
            return;
        }
        prev_vaddr = prev->n_blk;
        prev       = meta(space, prev_vaddr);
    }
}

void malloc_init()
{
    AddrSpace    *space     = kernel->currentThread->space;
    unsigned int  buf_vaddr = space->mallocbuffstart;
    blk_metadata *initial;

    ASSERT(space->pageTable[buf_vaddr / PageSize].valid);

    initial          = meta(space, buf_vaddr);
    initial->p_blk   = 0;
    initial->n_blk   = 0;
    initial->size    = MALLOC_BUFFER_SIZE - META_SIZE;
    initial->is_free = true;

    free_list_head  = buf_vaddr;
    alloc_list_head = buf_vaddr;
    initialized     = true;
    printf("malloc_init: ready, usable=%u bytes\n", MALLOC_BUFFER_SIZE - META_SIZE);
}

void *my_malloc(unsigned int bytes)
{
    AddrSpace    *space;
    unsigned int  curr_vaddr;
    blk_metadata *curr;

    if (!initialized) malloc_init();
    if (bytes == 0)   return NULL;

    bytes = (bytes + 3) & ~3u;  /* 4-byte align */
    space = kernel->currentThread->space;

    /* first-fit search */
    curr_vaddr = free_list_head;
    curr       = NULL;
    while (curr_vaddr != 0) {
        curr = meta(space, curr_vaddr);
        if (curr->size >= bytes) break;
        curr_vaddr = curr->n_blk;
        curr       = NULL;
    }

    if (curr == NULL) {
        printf("my_malloc(%u): out of memory\n", bytes);
        return NULL;
    }

    /* split if remainder is useful */
    if (curr->size >= bytes + META_SIZE + 4) {
        unsigned int  split_vaddr = curr_vaddr + META_SIZE + bytes;
        blk_metadata *split       = meta(space, split_vaddr);

        split->size    = curr->size - bytes - META_SIZE;
        split->is_free = true;
        split->p_blk   = curr_vaddr;
        split->n_blk   = curr->n_blk;

        if (curr->n_blk != 0)
            meta(space, curr->n_blk)->p_blk = split_vaddr;

        curr->n_blk = split_vaddr;
        curr->size  = bytes;

        fl_remove(curr_vaddr);
        fl_add(split_vaddr);
    } else {
        fl_remove(curr_vaddr);
    }

    curr->is_free = false;
    printf("my_malloc(%u) -> vaddr 0x%x\n", bytes, curr_vaddr + META_SIZE);
    return (void *)(curr_vaddr + META_SIZE);
}

void my_free(void *addr)
{
    AddrSpace    *space;
    unsigned int  data_vaddr;
    unsigned int  blk_vaddr;
    blk_metadata *blk;

    if (addr == NULL || !initialized) return;

    space      = kernel->currentThread->space;
    data_vaddr = (unsigned int)addr;
    blk_vaddr  = data_vaddr - META_SIZE;

    ASSERT(blk_vaddr >= space->mallocbuffstart);
    ASSERT(blk_vaddr <  space->mallocbuffend);

    blk = meta(space, blk_vaddr);

    if (blk->is_free) {
        printf("my_free: double free at 0x%x ignored\n", data_vaddr);
        return;
    }

    blk->is_free = true;

    /* coalesce with next */
    if (blk->n_blk != 0) {
        blk_metadata *next      = meta(space, blk->n_blk);
        unsigned int  next_vaddr = blk->n_blk;
        if (next->is_free) {
            fl_remove(next_vaddr);
            blk->size  += META_SIZE + next->size;
            blk->n_blk  = next->n_blk;
            if (next->n_blk != 0)
                meta(space, next->n_blk)->p_blk = blk_vaddr;
        }
    }

    /* coalesce with prev */
    if (blk->p_blk != 0) {
        blk_metadata *prev      = meta(space, blk->p_blk);
        unsigned int  prev_vaddr = blk->p_blk;
        if (prev->is_free) {
            fl_remove(prev_vaddr);
            prev->size += META_SIZE + blk->size;
            prev->n_blk = blk->n_blk;
            if (blk->n_blk != 0)
                meta(space, blk->n_blk)->p_blk = prev_vaddr;
            blk_vaddr = prev_vaddr;
        }
    }

    fl_add(blk_vaddr);
    printf("my_free: freed block at 0x%x\n", blk_vaddr);
}
