#include "syscall.h"
#include "sched.h"
#include "uart.h"
#include "task.h"

static int user_range_valid(uint64_t ptr, uint64_t size){
    if(size == 0)
        return 1;
    if(ptr < USER_START)
        return 0;
    if(ptr >= USER_END)
        return 0;
    
    if(size > USER_END - ptr)
        return 0;

    return 1;
}


// 0: sys_yield
static int64_t ksys_yield(void){
    schedule();
    return 0;
}

// 1: sys_write
static int64_t ksys_write(int fd, const char *buf, size_t count){
    if(fd != 1 && fd !=2) return -1; //support only stdout/stderr
    if(!buf) return -1;

    if(count > 4096)
        return -1;

    if(!user_range_valid((uint64_t)buf, count))
        return -1;

    for(size_t i = 0; i < count; i++){
        uart_putc(buf[i]);
    }
    return (int64_t)count;
}

// 2: sys_getpid
static int64_t ksys_getpid(void){
    task_struct_t *curr = sched_get_current_task();
    return curr ? (int64_t)curr->pid : -1;
}

// 3: sys_sleep
static int64_t ksys_sleep(uint64_t ticks){
    task_struct_t *curr = sched_get_current_task();
    if(!curr) return -1;

    if(ticks == 0){
        schedule();
        return 0;
    }

    curr->sleep_ticks = ticks;
    curr->state = TASK_BLOCKED;

    schedule();
    
    return 0;
}

// 4: sys_exit
static int64_t ksys_exit(int code){
    (void)code;
    task_exit();
    return 0;
}

//syscall pointer table
static void *syscall_table[MAX_SYSCALL];

void syscall_init(void){
    syscall_table[SYS_YIELD] = (void *)ksys_yield;
    syscall_table[SYS_WRITE] = (void *)ksys_write;
    syscall_table[SYS_GETPID] = (void *)ksys_getpid;
    syscall_table[SYS_SLEEP] = (void *)ksys_sleep;
    syscall_table[SYS_EXIT] = (void *)ksys_exit;

    uart_puts("[SYSCALL] System Call Table Initialized (5 Call Vectors Registerd)\n");
}

void syscall_dispatch(trap_frame_t *tf){
    uint64_t syscall_num = tf->x[8]; //AArch64 ABI

    if(syscall_num >= MAX_SYSCALL || !syscall_table[syscall_num]){
        uart_puts("[SYSCALL ERROR] Unknown Syscall Invoked: ");
        uart_put_hex(syscall_num);
        uart_puts("\n");

        tf->x[0] = (uint64_t)-1; // -ENOSYS
        return;
    }

    //syscall index mapping(x0, x1, x2, x3)
    uint64_t arg0 = tf->x[0];
    uint64_t arg1 = tf->x[1];
    uint64_t arg2 = tf->x[2];
    
    switch (syscall_num){
        case SYS_YIELD:
            tf -> x[0] = ((int64_t (*)(void))syscall_table[SYS_YIELD])();
            return;
        case SYS_WRITE:
            tf -> x[0] = ((int64_t (*)(int, const char *, size_t))syscall_table[SYS_WRITE])((int)arg0, (const char *)arg1, (size_t)arg2);
            return;
        case SYS_GETPID:
            tf -> x[0] = ((int64_t (*)(void))syscall_table[SYS_GETPID])();
            return;
        case SYS_SLEEP:
            tf -> x[0] = ((int64_t (*)(uint64_t))syscall_table[SYS_SLEEP])(arg0);
            return;
        case SYS_EXIT:
            ((int64_t (*)(int))syscall_table[SYS_EXIT])((int)arg0);
            while (1)
            {
                asm volatile("wfe");
            }
        default:
            tf -> x[0] = (uint64_t)-1;
            return;
    }
}


