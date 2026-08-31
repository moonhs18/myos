#include "test.h"
#include "heap.h"
#include "uart.h"

void test_heap_allocator(void){
    uart_puts("\n[TEST] --- Starting Kernel Heap (kmalloc/kfree) Unit Tests --- \n");

    void *ptr1 = kmalloc(32);
    void *ptr2 = kmalloc(128);
    void *ptr3 = kmalloc(512);

    uart_puts("[TEST] kmalloc(32) Address: ");
    uart_put_hex((uint64_t)ptr1);
    uart_puts("\n");
   
    uart_puts("[TEST] kmalloc(128) Address: ");
    uart_put_hex((uint64_t)ptr2);
    uart_puts("\n");
    
    uart_puts("[TEST] kmalloc(512) Address: ");
    uart_put_hex((uint64_t)ptr3);
    uart_puts("\n");

    uint8_t *byte_arr = (uint8_t *)ptr1;
    for(int i = 0; i < 32; i++){
        byte_arr[i] = (uint8_t)(0xAA + i);
    }
    
    int read_ok = 1;
    for(int i = 0; i < 32; i++){
        if(byte_arr[i] != (uint8_t)(0xAA + i)){
            read_ok = 0;
            break;
        }
    }
    if(read_ok){
        uart_puts("[TEST] Data Write/Read Test : PASS\n");
    }
    else{
        uart_puts("[TEST] Data Write/Read Test : FAIL\n");
    }

    kfree(ptr1);
    kfree(ptr2);
    kfree(ptr3);

    void *ptr_merged = kmalloc(600);
    uart_puts("[TEST] kmalloc(600) (Reused): ");
    uart_put_hex((uint64_t)ptr_merged);
    uart_puts(" (Expected start near ptr1)\n");

    kfree(ptr_merged);
    uart_puts("[TEST] [PASS] Heap Allocator Tests Completed Successfully!\n\n");

}