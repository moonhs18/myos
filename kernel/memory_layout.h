#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <stdint.h>

#define RAM_START       0x40000000ULL
#define RAM_SIZE        0x08000000ULL
#define RAM_END         (RAM_START +RAM_SIZE)

#define PAGE_SIZE       0x1000ULL

#define USER_START      0x47000000ULL
#define USER_SIZE       0x01000000ULL
#define USER_END        (USER_START + USER_SIZE)

#define USER_STACK_TOP  0x47FFF000ULL


#endif