#include "pmm.h"
#include "uart.h"
#include <stddef.h>


extern char __kernel_end[];

static uint64_t *bitmap = 0;
static uint64_t total_pages = 0;
static uint64_t bitmap_total_words = 0;
static uint64_t usable_start_addr = 0;

static inline void bitmap_set(uint64_t page_idx){
    bitmap[page_idx/64] |= (1ULL << (page_idx % 64));
}

static inline void bitmap_clear(uint64_t page_idx){
    bitmap[page_idx/64] &= ~(1ULL << (page_idx % 64));
}

static inline int bitmap_test(uint64_t page_idx){
    return (bitmap[page_idx / 64] & (1ULL <<(page_idx % 64))) != 0; // it is in there = 1, if not =0
}

static void pmm_memzero(void *dst, size_t size){
    uint8_t *p = (uint8_t *)dst;
    while (size--)
    {
        *p++ = 0;
    }
    
}

void pmm_init(void){
    total_pages = (RAM_END - RAM_START) / PAGE_SIZE;
    bitmap_total_words = (total_pages + 63) / 64;
    uint64_t bitmap_size_bytes = bitmap_total_words * 8;

    //locate bitmap -> kernel_end
    bitmap = (uint64_t *)(((uint64_t)__kernel_end + 7) & ~7ULL);
    
    //set all bits in the bitmap to 1
    for (uint64_t i = 0; i < bitmap_total_words; i++){
        bitmap[i] = ~0ULL;
    }

    uint64_t free_start  = (uint64_t)bitmap + bitmap_size_bytes;
    usable_start_addr = (free_start + PAGE_SIZE -1) & ~(PAGE_SIZE - 1);

    uint64_t start_page_idx = (usable_start_addr - RAM_START) / PAGE_SIZE; //to get page num
    for(uint64_t i = start_page_idx; i<total_pages; i++){
        bitmap_clear(i);
    }
}

void *pmm_alloc_pages(uint64_t count){
    if (count == 0) return 0;

    uint64_t start_page_idx = (usable_start_addr - RAM_START) / PAGE_SIZE;
    size_t found_count = 0;
    uint64_t found_start_idx = 0;

    for(uint64_t i = start_page_idx ; i < total_pages ; i++){
        if(!bitmap_test(i)){//bitmap is empty(not allocated)
            if(found_count == 0){
                found_start_idx = i;
            }
            found_count++;
            
            if(found_count == count){ //allocation is done
                for(uint64_t j = found_start_idx; j<found_start_idx + count ;j++){
                    bitmap_set(j);
                }
                void *addr = (void *)(RAM_START + (found_start_idx * PAGE_SIZE));
                pmm_memzero(addr, count * PAGE_SIZE);
                return addr; //return start_allocation addr
            }
        }
        else{//restart to find contiguous memory
            found_count =0;
        }
    }        
    return 0;
}


void *pmm_alloc_page(void){
    return pmm_alloc_pages(1);
}


void pmm_free_pages(void *ptr, size_t count){
    uint64_t addr = (uint64_t)ptr;

    if(ptr == 0 || addr < usable_start_addr || addr >= RAM_END || (addr & (PAGE_SIZE - 1)) !=0){
        return;
    }

    uint64_t page_idx = (addr - RAM_START) / PAGE_SIZE;
    for(size_t i = 0; i<count ; i++){
        if((page_idx + i) < total_pages){
            bitmap_clear(page_idx + i);
        }
    }
}

void pmm_free_page(void *ptr){
    return pmm_free_pages(ptr, 1);
}

uint64_t pmm_get_free_pages(void){
    uint64_t free_cnt = 0;
    uint64_t start_page_idx = (usable_start_addr - RAM_START) / PAGE_SIZE;
    for(uint64_t i =start_page_idx; i < total_pages; i++){
        if(!bitmap_test(i)){
            free_cnt++;
        }
    }
    return free_cnt;
}

