#include "elf.h"
#include "uart.h"
#include "pmm.h"
#include "mmu.h"
#include "heap.h"

static void elf_memcpy(void *dest, const void *src, size_t n){
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while(n--) *d++ = *s++;
}

static void elf_memset(void *ptr, int val, size_t n){
    uint8_t *p = (uint8_t *)ptr;
    while(n--) *p++ = (uint8_t)val;
}

int elf_validate(const void *elf_data, size_t size){
    if(!elf_data || size < sizeof(Elf64_Ehdr)){
        return 0;
    }

    const Elf64_Ehdr *hdr = (const Elf64_Ehdr *)elf_data;

    if(hdr->e_ident[0] != ELFMAG0 || hdr->e_ident[1] != ELFMAG1 || hdr->e_ident[2] != ELFMAG2 || hdr->e_ident[3] != ELFMAG3){
        uart_puts("[ELF ERROR] Invalid ELF Magic Number!\n");
        return 0;
    }

    if(hdr->e_ident[4] != ELFCLASS64){
        uart_puts("[ELF ERROR] Not a 64-bit ELF binary!\n");
        return 0;
    }

    if(hdr->e_machine != EM_AARCH64){
        uart_puts("[ELF ERROR] Target architecture is not AArch64!\n");
        return 0;
    }
    return 1;
}

uint64_t elf_load(const void *elf_data, size_t size){
    if(!elf_validate(elf_data, size)){
        return 0;
    }

    const Elf64_Ehdr *hdr = (const Elf64_Ehdr *)elf_data;
    const uint8_t *base = (const uint8_t *)elf_data;
    uint64_t *root_table = mmu_get_root_table();

    for(uint16_t i = 0; i < hdr->e_phnum; i++){
        const Elf64_Phdr *phdr = (const Elf64_Phdr *)(base + hdr->e_phoff + i * hdr->e_phentsize);

        if(phdr->p_type == PT_LOAD){
            uint64_t vaddr = phdr->p_vaddr;
            uint64_t memsz = phdr->p_memsz;
            uint64_t filesz = phdr->p_filesz;
            uint64_t offset = phdr->p_offset;

            size_t page_count = (memsz + PAGE_SIZE - 1) / PAGE_SIZE;

            for(size_t p=0; p <page_count; p++){
                uint64_t curr_va = vaddr + p *PAGE_SIZE;
                uint64_t pa = mmu_translate(root_table, curr_va);

                if(pa==0){
                    void *new_phys = pmm_alloc_page();
                    if(!new_phys){
                        uart_puts("[ELF ERROR] Out of Memory mapping ELF segment!\n");
                        return 0;
                    }
                    elf_memset(new_phys, 0, PAGE_SIZE);
                    mmu_map_page(root_table, curr_va, (uint64_t)new_phys, MMU_FLAG_USER_CODE);
                    mmu_tlb_flush();
                    pa = (uint64_t)new_phys;
                }

                size_t page_off = p * PAGE_SIZE;
                if(page_off < filesz){
                    size_t copy_len = filesz - page_off;
                    if(copy_len > PAGE_SIZE) copy_len = PAGE_SIZE;
                    elf_memcpy((void *)pa, base + offset + page_off, copy_len);
                }

            }
        }
    }

    uart_puts("[ELF LOADER] Executable loaded successfully. Entry point = ");
    uart_put_hex(hdr->e_entry);
    uart_puts("\n");

    return hdr->e_entry;
}

