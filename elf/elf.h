#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

#define EI_NIDENT       16


//ELF Magic Number & Header Identifiers
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

#define ELFCLASS64      2
#define ELFDATA2LSB     1
#define EM_AARCH64      183
#define ET_EXEC         2

//Program Header Types & Flags
#define PT_LOAD         1
#define PF_X            1
#define PF_W            2
#define PF_R            4

typedef struct
{
    unsigned char   e_ident[EI_NIDENT];
    uint16_t        e_type;
    uint16_t        e_machine;
    uint32_t        e_version;
    uint64_t        e_entry;
    uint64_t        e_phoff;
    uint64_t        e_shoff;
    uint32_t        e_flags;
    uint16_t        e_ehsize;
    uint16_t        e_phentsize;
    uint16_t        e_phnum;
    uint16_t        e_shentsize;
    uint16_t        e_shnum;
    uint16_t        e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
}Elf64_Phdr;

int         elf_validate(const void *elf_data, size_t size);
uint64_t    elf_load(const void *elf_data, size_t size);

#endif