#include "uart.h"
#include "exception.h"
#include "pmm.h"
#include "heap.h"
#include "mmu.h"

#ifdef ENABLE_TESTS
#include "test.h"
#endif

extern char __stack_top[];
extern char __kernel_end[];

void kernel_main(void) {
    
    uart_init();
    uart_puts("\n=======================================\n");
    uart_puts("  MiniOS (ARM64) Successfully Booted!  \n");
    uart_puts("=======================================\n");

    exception_init();
    uart_puts("[OK] Exception Vector Table Registered.\n");

    pmm_init();
    uart_puts("[OK] Physical Memory Manager (PMM) Initialized.\n");

    heap_init();
    uart_puts("[OK] Kernel Heap Allocator (kmalloc) Initialized.\n");

    mmu_init();
    uart_puts("[OK] ARM64 MMU & 4-Level Paging Activated.\n");

#ifdef ENABLE_TESTS
    uart_puts(">> Running Unit Tests...\n");
    test_pmm_allocator();
    test_heap_allocator();
    test_mmu_virtual_memory();
#endif
    uart_puts(">> Kernel is now in idle state.\n");

    while (1)
    {
        asm volatile("wfe");
    }

}