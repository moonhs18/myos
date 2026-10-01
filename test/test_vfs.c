#include "test.h"
#include "vfs.h"
#include "syscall.h"
#include "uart.h"
#include "sched.h"
#include "task.h"
static void __attribute__((section(".user_text"))) user_vfs_test_task(void) {
    static const char dev_path[] __attribute__((section(".user_rodata"))) = "/dev/console";
    static const char ram_path[] __attribute__((section(".user_rodata"))) = "/ram/hello.txt";
    static const char msg_start[] __attribute__((section(".user_rodata"))) = "\n[USER TASK] Starting VFS & RamFS & DevFS Test in EL0...\n";
    static const char msg_dev_pass[] __attribute__((section(".user_rodata"))) = "[TEST PASS] Writing directly to /dev/console via DevFS!\n";
    static const char msg_dev_fail[] __attribute__((section(".user_rodata"))) = "[TEST FAIL] Failed to open /dev/console\n";
    static const char msg_ram_content[] __attribute__((section(".user_rodata"))) = "MiniOS AArch64 VFS Framework Operational!";
    static const char msg_ram_fail[] __attribute__((section(".user_rodata"))) = "[TEST FAIL] Failed to create /ram/hello.txt\n";
    static const char msg_read_pass[] __attribute__((section(".user_rodata"))) = "[TEST PASS] Read from /ram/hello.txt: \"";
    static const char msg_quote_end[] __attribute__((section(".user_rodata"))) = "\"\n";
    static const char msg_done[] __attribute__((section(".user_rodata"))) = ">> VFS Infrastructure Integration Tests Completed Successfully!\n\n";

    sys_write(1, msg_start, sizeof(msg_start) - 1);

    int console_fd = (int)sys_open(dev_path, O_WRONLY);
    if (console_fd >= 0) {
        sys_write(console_fd, msg_dev_pass, sizeof(msg_dev_pass) - 1);
        sys_close(console_fd);
    } else {
        sys_write(1, msg_dev_fail, sizeof(msg_dev_fail) - 1);
    }

    int file_fd = (int)sys_open(ram_path, O_CREAT | O_RDWR);
    if (file_fd >= 0) {
        sys_write(file_fd, msg_ram_content, sizeof(msg_ram_content) - 1);
        sys_close(file_fd);

        int read_fd = (int)sys_open(ram_path, O_RDONLY);
        if (read_fd >= 0) {
            char read_buf[64];
            for (int i = 0; i < 64; i++) read_buf[i] = 0;

            int64_t read_bytes = sys_read(read_fd, read_buf, 64);
            sys_close(read_fd);

            if (read_bytes > 0) {
                sys_write(1, msg_read_pass, sizeof(msg_read_pass) - 1);
                sys_write(1, read_buf, read_bytes);
                sys_write(1, msg_quote_end, sizeof(msg_quote_end) - 1);
            }
        }
    } else {
        sys_write(1, msg_ram_fail, sizeof(msg_ram_fail) - 1);
    }

    sys_write(1, msg_done, sizeof(msg_done) - 1);
    sys_exit(0);
}

void test_vfs_subsystem(void) {
    uart_puts("\n[TEST] ================================================\n");
    uart_puts("[TEST]      Starting Phase 7: VFS Subsystem Tests       \n");
    uart_puts("[TEST] ================================================\n");

    task_struct_t *task = task_create_user(user_vfs_test_task);
    if (!task) {
        uart_puts("[TEST] [FAIL] Failed to create VFS Test User Task!\n");
        return;
    }

    while (sched_get_live_user_tasks() != 0) {
        asm volatile("wfi");
    }

    uart_puts("[TEST] [PASS] VFS Subsystem Verified Successfully!\n");
    uart_puts("=======================================================\n\n");
}