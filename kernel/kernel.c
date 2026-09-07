#include "uart.h"
#include "exception.h"
#include "pmm.h"
#include "heap.h"
#include "mmu.h"
#include "gic.h"
#include "timer.h"
#include "sched.h"

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
    
    gic_init();
    uart_puts("[OK] GICv2 Interrupt Controller Initialized.\n");

    timer_init(100);
    uart_puts("[OK] ARM Generic Timer Initialized.\n");
    
    sched_init();
    uart_puts("[OK] Preemptive Scheduler Initialized.\n");

    enable_irq();
    uart_puts("[OK] CPU Global Interrupts Unmasked.\n");

#ifdef ENABLE_TESTS
    test_timer_ticks();
    test_scheduler_preemption();
#endif
    uart_puts(">> Kernel is now in idle state.\n");

    while (1)
    {
        asm volatile("wfi");
    }

}