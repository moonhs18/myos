#include "uart.h"

// linker.ld에 정의된 심볼 참조
extern char __stack_top[];
extern char __kernel_end[];

void kernel_main(void) {
    uart_init();

    uart_puts("\n==============================\n");
    uart_puts("[MiniOS] Booting AArch64 Kernel...\n");
    uart_puts("==============================\n");

    uart_puts("Kernel End Address : ");
    uart_put_hex((uint64_t)__kernel_end);
    uart_puts("\n");

    uart_puts("Kernel Stack Top   : ");
    uart_put_hex((uint64_t)__stack_top);
    uart_puts("\n");

    uart_puts("[MiniOS] Ready for Exception Vector setup.\n");

    while (1) {
        // Halt state
    }
}