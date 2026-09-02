#include <test.h>
#include <mmu.h>
#include <pmm.h>
#include <uart.h>

static void print_subtest(const char *name, int pass){
    uart_puts("  ->  ");
    uart_puts(name);
    if(pass)    uart_puts(": [PASS]\n");
    else        uart_puts(": [FAIL]\n");
}



void test_mmu_virtual_memory(void){
    uart_puts("\n[TEST] ================================================\n");
    uart_puts("[TEST]       Starting Comprehensive MMU Unit Tests     \n");
    uart_puts("[TEST] ================================================\n");

    uint64_t *root = mmu_get_root_table();
    int overall_pass = 1;

    //page table walk test
    uart_puts("[1] Page Table Hardware Walk Test\n\n");

    uint64_t pa_ram = mmu_translate(root, 0x40000000);
    uint64_t pa_uart = mmu_translate(root, 0x09000000);                                      
    uint64_t pa_unmapped = mmu_translate(root, 0x55000000);
    

    int test1_ok = (pa_ram == 0x40000000) && (pa_uart == 0x09000000) && (pa_unmapped == 0);

    print_subtest("RAM/UART Translation & Unmapped Space Detection", test1_ok);
    if (!test1_ok) overall_pass = 0;


    //Non Identity mapping test
    uart_puts("[2] Non-Identity Dynamic Mapping Test (VA != PA)\n");
    void *phys_frame = pmm_alloc_page();
    uint64_t test_va = 0x50000000ULL;

    mmu_map_page(root, test_va, (uint64_t)phys_frame, MMU_FLAG_RAM);
    

    mmu_tlb_flush();

    *(volatile uint64_t *)test_va = 0xFEEDC0FFEE123456ULL;
    
    uint64_t direct_read = *(volatile uint64_t *)phys_frame;
    int test2_ok = (direct_read == 0xFEEDC0FFEE123456ULL);

    *(volatile uint64_t *)phys_frame = 0xABCDEF0123456789ULL;
    if (*(volatile uint64_t *)test_va != 0xABCDEF0123456789ULL) {
        test2_ok = 0;
    }
    print_subtest("Bi-directional Translation Consistency (VA <-> PA)", test2_ok);
    if (!test2_ok) overall_pass = 0;

    // stress test for crossing 4KB page boundaries
    uart_puts("[3] Multi-Page Boundary Stress Test\n");
    void *multi_phys = pmm_alloc_pages(2);
    uint64_t multi_va = 0x60000000ULL;

    mmu_map_range(root, multi_va, (uint64_t)multi_phys, PAGE_SIZE * 2, MMU_FLAG_RAM);
    mmu_tlb_flush();

    volatile uint64_t *cross_boundary_ptr = (volatile uint64_t *)(multi_va + PAGE_SIZE - 4);
    *cross_boundary_ptr = 0xAABBCCDDEEFF0011ULL;

    int test3_ok = (*cross_boundary_ptr == 0xAABBCCDDEEFF0011ULL);
    print_subtest("Cross-Page Boundary 64-bit Memory Access", test3_ok);
    if (!test3_ok) overall_pass = 0;


    //page remapping and TLB invalidation
    uart_puts("[4] Dynamic Remapping & TLB Invalidation Test\n");
    void *new_phys = pmm_alloc_page();
    *(volatile uint64_t *)new_phys = 0x777788889999AAAAULL;

    mmu_map_page(root, test_va, (uint64_t)new_phys, MMU_FLAG_RAM);
    mmu_tlb_flush();

    int test4_ok = (*(volatile uint64_t *)test_va == 0x777788889999AAAAULL);
    print_subtest("TLB Flush and Remapped Page Visibility", test4_ok);
    if (!test4_ok) overall_pass = 0;


    //resource deallocation and cleanup
    pmm_free_page(phys_frame);
    pmm_free_pages(multi_phys, 2);
    pmm_free_page(new_phys);

    uart_puts("-------------------------------------------------------\n");
    if (overall_pass) {
        uart_puts("[TEST] [PASS] ALL MMU & PAGING TESTS COMPLETED SUCCESSFULLY!\n");
    } else {
        uart_puts("[TEST] [FAIL] MMU UNIT TESTS FAILED!\n");
    }
    uart_puts("=======================================================\n\n");

}
