#include "heap.h"
#include "pmm.h"
#include "uart.h"

static block_header_t *heap_start = 0;

#define INITIAL_HEAP_PAGES 4

void heap_init(void){
    void *initial_mem = pmm_alloc_pages(INITIAL_HEAP_PAGES);
    if(!initial_mem){
        uart_puts("[HEAP ERROR] Failed to allocate physical memory for Heap!\n");
        return;
    }

    heap_start = (block_header_t *)initial_mem;
    heap_start->size = (INITIAL_HEAP_PAGES * PAGE_SIZE) - HEADER_SIZE;
    heap_start->is_free = 1;
    heap_start->next = 0;
}


void *kmalloc(size_t size){
    if(size == 0) return 0;

    size_t aligned_size = ALIGN_UP16(size);//aligned 16bytes
    block_header_t *curr = heap_start;

    while(curr){
        if(curr->is_free && (curr->size) >= aligned_size){
            if((curr->size) >= aligned_size + HEADER_SIZE + HEAP_ALIGNMENT){
                block_header_t *new_block = (block_header_t *)((uint8_t *)curr + HEADER_SIZE + aligned_size);
                new_block->size = curr->size - aligned_size - HEADER_SIZE;
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = aligned_size;
                curr->next = new_block;
            }
            curr->is_free = 0;

            return (void *)((uint8_t *)curr + HEADER_SIZE);
        }
        curr = curr->next;
    }

    size_t pages_needed = (aligned_size + HEADER_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    void *new_pages = pmm_alloc_pages(pages_needed);
    if(!new_pages){
        uart_puts("[HEAP ERROR] Out of Memory in kmalloc!\n");
        return 0;
    }

    block_header_t *expanded_block = (block_header_t *)new_pages;
    expanded_block->size = (pages_needed * PAGE_SIZE) - HEADER_SIZE;
    expanded_block->is_free=0;
    expanded_block->next = 0;

    curr = heap_start;
    while (curr->next)
    {
        curr = curr->next;
    }
    curr->next = expanded_block;

    return (void *)((uint8_t *)expanded_block + HEADER_SIZE);
}

void kfree(void *ptr){
    if(!ptr) return;

    block_header_t *header = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    header->is_free = 1;

    block_header_t *curr = heap_start;
    while(curr && curr->next){
        if(curr->is_free && curr->next->is_free){
            if((uint8_t *)curr + HEADER_SIZE + curr->size == (uint8_t *)curr->next){
                curr->size += HEADER_SIZE + curr->next->size;
                curr->next = curr->next->next;
                continue;
            }
        }
        curr = curr->next;
    }

}