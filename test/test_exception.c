#include "uart.h"
#include <stdint.h>

void test_exception_abort(void){
    uart_puts("\n[TEST]------------------------------------\n");
    uart_puts("\n[TEST] Triggering Intentional Data Abort...\n");
    uart_puts("\n[TEST]------------------------------------\n");

    volatile uint64_t *invalid_addr = (volatile uint64_t *)0XDEADBEEF0000;
    *invalid_addr = 0X12345678;

    uart_puts("[FAIL] This message must NOT be printed.\n");
}