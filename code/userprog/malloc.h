#ifndef MALLOC_H
#define MALLOC_H

#include "copyright.h"

#define MALLOC_BUFFER_PAGES 32
#define MALLOC_BUFFER_SIZE  (MALLOC_BUFFER_PAGES * 128)  /* 128 = PageSize */
#define META_SIZE           ((unsigned int)sizeof(struct blk_metadata))

struct blk_metadata {
    unsigned int p_blk;    /* virtual addr of previous block, 0 = none */
    unsigned int n_blk;    /* virtual addr of next block,     0 = none */
    unsigned int size;     /* bytes of user data after this header     */
    bool         is_free;
};

void  malloc_init();
void *my_malloc(unsigned int bytes);
void  my_free(void *addr);

#endif /* MALLOC_H */
