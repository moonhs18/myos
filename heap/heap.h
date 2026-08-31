#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_ALIGNMENT 16
#define ALIGN_UP16(size) (((size) + (HEAP_ALIGNMENT - 1)) & ~(HEAP_ALIGNMENT - 1))

typedef struct block_header{
    size_t size;
    int is_free;
    struct block_header *next;
    uint64_t padding;
} block_header_t;

#define HEADER_SIZE (sizeof(block_header_t))

void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif