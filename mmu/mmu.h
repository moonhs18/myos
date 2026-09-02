#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SHIFT      12
#define PAGE_SIZE       4096
#define TABLE_ENTRIES   512
#define VA_MASK_9BIT    0x1FF //0b 0001 1111 1111

//macro to extract page table index from VA
#define L0_INDEX(va)    (((va) >> 39) & VA_MASK_9BIT)
#define L1_INDEX(va)    (((va) >> 30) & VA_MASK_9BIT)
#define L2_INDEX(va)    (((va) >> 21) & VA_MASK_9BIT)
#define L3_INDEX(va)    (((va) >> 12) & VA_MASK_9BIT)

#define MMU_DESCRIPTOR_TABLE    0x3ULL //table descriptor

#define MMU_DESCRIPTOR_PAGE     0x3ULL //valid 4KB page
#define MMU_ATTR_DEVICE_INDEX   (0ULL << 2) //Attr 0(Device - nGnRnE)
#define MMU_ATTR_NORMAL_INDEX   (1ULL << 2) //Attr 1(Normal Cachable)
#define MMU_ACCESS_FLAG         (1ULL << 10) //bit[10] = 1
#define MMU_SH_INNER_SHAREABLE  (3ULL << 8) //bit[9:8] = 0b11
#define MMU_AP_KERNEL_RW        (0ULL << 6) //bit[7:6] = 0b00 (EL1 Read/Write)
#define MMU_EXECUTE_NEVER       (1ULL << 54)//bit[54] = 1

#define MMU_FLAG_RAM        (MMU_DESCRIPTOR_PAGE | MMU_ATTR_NORMAL_INDEX | \
                            MMU_ACCESS_FLAG | MMU_SH_INNER_SHAREABLE | MMU_AP_KERNEL_RW)

#define MMU_FLAG_DEVICE     (MMU_DESCRIPTOR_PAGE | MMU_ATTR_DEVICE_INDEX | \
                            MMU_ACCESS_FLAG | MMU_AP_KERNEL_RW | MMU_EXECUTE_NEVER)

void mmu_init(void);
void mmu_map_page(uint64_t *l0_table, uint64_t va, uint64_t pa, uint64_t flags);
void mmu_map_range(uint64_t *l0_table, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t flags);

uint64_t *mmu_get_root_table(void);
uint64_t mmu_translate(uint64_t *l0_table, uint64_t va);
void mmu_tlb_flush(void);


extern void arm64_mmu_enable(uint64_t l0_table_pa);

#endif