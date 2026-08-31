#include "test.h"
#include "pmm.h"
#include "uart.h"

void test_pmm_allocator(void){
    uart_puts("\n[TEST] --- starting PMM Unit Tests --- \n");

    uint64_t initial_free = pmm_get_free_pages();
    uart_puts("[TEST] Initial Free pages:");
    uart_put_hex(initial_free);
    uart_puts("\n");

    void *page1 = pmm_alloc_page();
    void *page2 = pmm_alloc_page();

    uart_puts("[TEST] Allocated page 1: ");
    uart_put_hex((uint64_t)page1);
    uart_puts("\n");

    uart_puts("[TEST] Allocated page 2: ");
    uart_put_hex((uint64_t)page2);
    uart_puts("\n");

    pmm_free_page(page1);
    void *page1_realloc = pmm_alloc_page();
    uart_puts("[TEST] Re-allocated page: ");
    uart_put_hex((uint64_t)page1_realloc);
    uart_puts(" (Expected same as Page 1)\n");


    void *multi_pages = pmm_alloc_pages(4);
    uart_puts("[TEST] Allocated 4 pages: ");
    uart_put_hex((uint64_t)multi_pages);
    uart_puts("\n");

    pmm_free_pages(multi_pages, 4);
    pmm_free_page(page2);
    pmm_free_page(page1_realloc);

    uint64_t final_free = pmm_get_free_pages();
    uart_puts("[TEST] Final Free Pages  : ");
    uart_put_hex(final_free);
    uart_puts("\n");

    if (initial_free == final_free) {
        uart_puts("[TEST] [PASS] PMM All Tests Passed Successfully!\n\n");
    } else {
        uart_puts("[TEST] [FAIL] Memory Leak Detected in PMM!\n\n");
    }

}