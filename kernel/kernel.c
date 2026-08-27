#include "uart.h"
#include "exception.h"

// linker.ld에 정의된 심볼 참조
extern char __stack_top[];
extern char __kernel_end[];

void kernel_main(void) {
    uart_init();

    uart_puts("\n==============================\n");
    uart_puts("[MiniOS] Booting AArch64 Kernel...\n");
    uart_puts("==============================\n");
    
    exception_init();
    uart_puts("[OK] Exception Vetor Table (VBAR_EL1) Registered. \n\n");
    
    uart_puts("[TEST] Triggering intentional Data Abort...\n");
    volatile uint64_t *invalid_addr = (volatile uint64_t *)0xDEADBEEF0000;
    *invalid_addr = 0x12345678;
    
    uart_puts("This message must NOT be printed.\n");

    while (1);
}