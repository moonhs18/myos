#include "test.h"
#include "sched.h"
#include "timer.h"
#include "uart.h"

static volatile int task_a_counter = 0;
static volatile int task_b_counter = 0;

static void task_worker_a(void) {
    while (task_a_counter < 3) {
        uart_puts("  [Task A] Executing... tick=");
        uart_put_hex(timer_get_ticks());
        uart_puts("\n");
        task_a_counter++;
        timer_sleep_ticks(10); // 100ms 대기 (타이머 인터럽트에 의한 선점 유도)
    }
    uart_puts("  [Task A] Work Finished -> Exiting.\n");
}

static void task_worker_b(void) {
    while (task_b_counter < 3) {
        uart_puts("  [Task B] Executing... tick=");
        uart_put_hex(timer_get_ticks());
        uart_puts("\n");
        task_b_counter++;
        timer_sleep_ticks(10);
    }
    uart_puts("  [Task B] Work Finished -> Exiting.\n");
}

void test_scheduler_preemption(void) {
    uart_puts("\n[TEST] ================================================\n");
    uart_puts("[TEST]    Starting Multitasking & Scheduler Tests      \n");
    uart_puts("[TEST] ================================================\n");

    task_create(task_worker_a);
    task_create(task_worker_b);

    uart_puts("[TEST] Tasks Created. Waiting for Preemptive Context Switches...\n");

    // 두 태스크가 모두 3회 이상 실행을 마치고 종료될 때까지 대기
    while (task_a_counter < 3 || task_b_counter < 3) {
        asm volatile("wfi");
    }

    uart_puts("[TEST] [PASS] Preemptive Multitasking Successfully Verified!\n");
    uart_puts("=======================================================\n\n");
}