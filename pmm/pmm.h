#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE   4096
#define RAM_START   0x40000000
#define RAM_SIZE    0x08000000
#define RAM_END     (RAM_START + RAM_SIZE)


void pmm_init(void);
void *pmm_alloc_page(void);
void *pmm_alloc_pages(size_t count);
void pmm_free_page(void *ptr);
void pmm_free_pages(void *ptr, size_t count);

uint64_t pmm_get_free_pages(void);

#endif