#include "mmu.h"
#include "pmm.h"
#include "uart.h"

static uint64_t *l0_root = 0;

static uint64_t* get_or_create_table(uint64_t *parent_table, uint64_t index){
    uint64_t entry = parent_table[index];

    if(entry & 0x1){// If page aleady exists >>  return addr
        return (uint64_t *)(entry & ~0xFFFULL);
    }

    void *new_table = pmm_alloc_page();
    if(!new_table){
        uart_puts("[MMU ERROR] Out of Memory while allocting page table!\n");
        return 0;
    }

    parent_table[index] = ((uint64_t)new_table & ~0xFFFULL) | MMU_DESCRIPTOR_TABLE;
    return (uint64_t *)new_table;
}

void mmu_map_page(uint64_t *l0_table, uint64_t va, uint64_t pa, uint64_t flags){
    uint64_t l0_idx = L0_INDEX(va);
    uint64_t l1_idx = L1_INDEX(va);
    uint64_t l2_idx = L2_INDEX(va);
    uint64_t l3_idx = L3_INDEX(va);

    uint64_t *l1_table = get_or_create_table(l0_table, l0_idx);
    uint64_t *l2_table = get_or_create_table(l1_table, l1_idx);
    uint64_t *l3_table = get_or_create_table(l2_table, l2_idx);

    l3_table[l3_idx] = (pa & ~0xFFFULL) | flags;
}

void mmu_map_range(uint64_t *l0_table, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t flags){
    uint64_t va = va_start & ~0xFFFULL;
    uint64_t pa = pa_start & ~0xFFFULL;
    size_t pages = (size + PAGE_SIZE -1) / PAGE_SIZE;

    for(size_t i = 0; i < pages; i++){
        mmu_map_page(l0_table, va, pa, flags);
        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }
}

void mmu_init(void){
    l0_root = (uint64_t *)pmm_alloc_page();
    if(!l0_root){
        uart_puts("[MMU ERROR] Failed to allocate L0 Root page Table!\n");
        return;
    }
    
    //UART MMIO mapping
    mmu_map_range(l0_root, 0x09000000, 0x09000000, 0x10000, MMU_FLAG_DEVICE);

    //kernel ram identity mapping 
    mmu_map_range(l0_root, RAM_START, RAM_START, RAM_SIZE, MMU_FLAG_RAM);

    //set system register and run mmu
    arm64_mmu_enable((uint64_t)l0_root);
}