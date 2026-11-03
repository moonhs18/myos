#include "uart.h"
#include "exception.h"
#include "pmm.h"
#include "heap.h"
#include "mmu.h"
#include "gic.h"
#include "timer.h"
#include "sched.h"
#include "syscall.h"
#include "vfs.h"
#include "ramfs.h"
#include "devfs.h"
#include "shell.h"
#include "task.h"

#ifdef ENABLE_TESTS
#include "test.h"
#endif

extern char __stack_top[];
extern char __kernel_end[];

static inline uint64_t read_sp(void){
    uint64_t sp;
    asm volatile("mov %0, sp" : "=r"(sp));
    return sp;
}

void kernel_main(void) {
    
    //Keep interrupts disabled during initialization
    disable_irq();

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
    
    sched_init(read_sp());
    uart_puts("[OK] Preemptive Scheduler Initialized.\n");

    syscall_init();
    uart_puts("[OK] System Call Table Initialized.\n");

    vfs_init();
    uart_puts("[OK] Virtual File System Infrastructure Initialized.\n");

    ramfs_init();
    uart_puts("[OK] RamFS Initialized and Mounted on /ram.\n");

    devfs_init();
    uart_puts("[OK] DevFS Initialized and Mounted on /dev.\n");

    //enable_irq();
    //uart_puts("[OK] CPU Global Interrupts Unmasked.\n");


#ifdef ENABLE_TESTS
    test_timer_ticks(); 
    test_scheduler_preemption();
    test_user_mode();
    test_vfs_subsystem();
    test_shell_and_apps();
#else
    uart_puts(">>Launching EL0 Interactive Shell Process...\n");
    task_struct_t *shell_task =
    task_create_user_elf("shell", shell_main);

    if (shell_task == NULL) {
        uart_puts("[ERROR] Failed to create shell task.\n");
    } else {
        uart_puts("[DEBUG] Starting shell without timer preemption.\n");

        schedule();
    }



#endif
    uart_puts(">> Kernel is now in idle state.\n");

    while (1)
    {
        asm volatile("wfi");
    }

}