#include "uart.h"
#include "exception.h"

#ifdef ENABLE_TESTS
#include "test.h"
#endif

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

#ifdef ENABLE_TESTS
    uart_puts(">> Running Self-Diagnostics / Unit Tests...\n");
    test_exception_abort();
#endif
    uart_puts(">> Kernel is now in idle state.\n");

    while (1)
    {
        asm volatile("wfe");
    }

}