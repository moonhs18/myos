#include "test.h"
#include "syscall.h"
#include "sched.h"
#include "uart.h"
#include "task.h"

//user RO data
#define USER_RODATA \
    __attribute__((section(".user_rodata"),aligned(16)))

static const char user1_msg1[] USER_RODATA = "[User Task 1] Running in EL0.\n";
static const char user1_msg2[] USER_RODATA = "[User Task 1] Yielding CPU.\n";
static const char user1_msg3[] USER_RODATA = "[User Task 1] Back from yield. Sleeping 5 ticks.\n";
static const char user1_msg4[] USER_RODATA = "[User Task 1] Wokr up. Exiting.\n";

static const char user2_msg1[] USER_RODATA = "[User Task 1] Running in EL0.\n";
static const char user2_msg2[] USER_RODATA = "[User Task 1] Work finished. Exiting.\n";

//user code

#define USER_TEXT \
    __attribute__((section(".user_text"),aligned(16)))

static void user_process_1(void) USER_TEXT;
static void user_process_2(void) USER_TEXT;


static void user_process_1(void){
    //TEST SYS_GETPID
    int64_t pid = sys_getpid();
    (void)pid;

    //TEST SYS_WRITE
    sys_write(1,user1_msg1,sizeof(user1_msg1)-1);

    //TEST SYS_YIELD
    sys_write(1,user1_msg2,sizeof(user1_msg2)-1);
    sys_yield();

    
    //TEST SYS_SLEEP
    sys_write(1,user1_msg3,sizeof(user1_msg3)-1);
    sys_sleep(5);
    
    //TEST WAKE_UP
    sys_write(1,user1_msg4,sizeof(user1_msg4)-1);

    //TEST SYS_EXIT
    sys_exit(0);
}

static void user_process_2(void){
    //TEST SYS_GETPID
    int64_t pid = sys_getpid();
    (void)pid;

    //TEST SYS_WRITE
    sys_write(1,user2_msg1,sizeof(user2_msg1)-1);

    sys_write(1,user2_msg2,sizeof(user2_msg2)-1);
  
    //TEST SYS_EXIT
    sys_exit(0);
}

void test_user_mode(void){
    uart_puts("\n[TEST] ================================================\n");

    uart_puts("[TEST]      Starting Phase 6: User Mode (EL0) & SVC\n");

    uart_puts("[TEST] ================================================\n");


    task_struct_t *task1 =task_create_user(user_process_1);

    task_struct_t *task2 =task_create_user(user_process_2);


    if(!task1 || !task2)
    {
        uart_puts("[TEST] [FAIL] Failed to create User Tasks!\n");
        return;
    }


    uart_puts("[TEST] User Tasks Created.\n");

    uart_puts("[TEST] Waiting for User Tasks...\n");


    while(sched_get_live_user_tasks() != 0)
    {
        asm volatile("wfi");
    }


    uart_puts("[TEST] [PASS] User Mode (EL0) & System Calls Verified!\n");

    uart_puts("=======================================================\n\n");
}