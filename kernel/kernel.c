#include "uart.h"
#include "exception.h"
#include "pmm.h"

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

    uart_puts("\n--- System Memory Info ---\n");
    uart_puts("Kernel End Address : ");
    uart_put_hex((uint64_t)__kernel_end);
    uart_puts("\n");
    uart_puts("Kernel Stack Top   : ");
    uart_put_hex((uint64_t)__stack_top);
    uart_puts("\n");
    uart_puts("Free Physical Pages: ");
    uart_put_hex(pmm_get_free_pages());
    uart_puts("\n--------------------------\n\n");

#ifdef ENABLE_TESTS
    uart_puts(">> Running Unit Tests...\n");
    test_pmm_allocator();
#endif
    uart_puts(">> Kernel is now in idle state.\n");

    while (1)
    {
        asm volatile("wfe");
    }

}