#include "test.h"
#include "timer.h"
#include "uart.h"

void test_timer_ticks(void) {
    uart_puts("\n[TEST] ================================================\n");
    uart_puts("[TEST]    Starting Hardware Timer & GICv2 Unit Tests   \n");
    uart_puts("[TEST] ================================================\n");

    uint64_t start_tick = timer_get_ticks();
    uart_puts("[TEST] Waiting for 10 Timer Ticks (~100ms)...\n");

    // 10틱(약 100ms) 대기
    timer_sleep_ticks(10);
    

    uint64_t end_tick = timer_get_ticks();
    uart_puts("[TEST] Start Tick: ");
    uart_put_hex(start_tick);
    uart_puts(" -> End Tick: ");
    uart_put_hex(end_tick);
    uart_puts("\n");

    if (end_tick >= start_tick + 10) {
        uart_puts("[TEST] [PASS] Hardware Timer & Interrupt Processing OK!\n");
    } else {
        uart_puts("[TEST] [FAIL] Timer Ticks did not increase properly!\n");
    }
    uart_puts("=======================================================\n\n");
}