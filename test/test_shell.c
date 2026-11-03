#include "test.h"
#include "uart.h"
#include "sched.h"
#include "shell.h"
#include "vfs.h"
#include "task.h"

void test_shell_and_apps(void) {
    uart_puts("\n[TEST] ================================================\n");
    uart_puts("[TEST]    Starting Phase 8: Shell & ELF Loader Tests    \n");
    uart_puts("[TEST] ================================================\n");

    file_t *f = vfs_open("/ram/welcome.txt", O_CREAT | O_RDWR);
    if (f) {
        const char *msg = "Welcome to MiniOS AArch64 System Test File!";
        vfs_write(f, msg, 43);
        vfs_close(f);
    }

    uart_puts("[TEST] Launching EL0 Interactive Shell Process...\n");
    task_struct_t *shell_task = task_create_user_elf("shell", shell_main);
    if (!shell_task) {
        uart_puts("[TEST] [FAIL] Failed to launch Shell Task!\n");
        return;
    }

    uart_puts("[TEST] [PASS] Phase 8 Infrastructure Loaded Successfully!\n");
    uart_puts("=======================================================\n\n");
}