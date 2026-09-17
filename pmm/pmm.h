#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#include "memory_layout.h"

void pmm_init(void);
void *pmm_alloc_page(void);
void *pmm_alloc_pages(size_t count);
void pmm_free_page(void *ptr);
void pmm_free_pages(void *ptr, size_t count);

uint64_t pmm_get_free_pages(void);

#endif