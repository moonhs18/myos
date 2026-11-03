#include "syscall.h"
#include "sched.h"
#include "uart.h"
#include "task.h"
#include "vfs.h"
#include "elf.h"
#include "heap.h"

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
    uart_puts("[SYSCALL] System Call Table Initialized (5 Call Vectors Registerd)\n");
}

void syscall_dispatch(trap_frame_t *tf) {
    uint64_t syscall_num = tf->x[8];
    task_struct_t *current = sched_get_current_task();

    switch (syscall_num) {
        case SYS_YIELD:
            sched_yield();
            tf->x[0] = 0;
            break;

        case SYS_WRITE: {
            int fd = (int)tf->x[0];
            const char *buf = (const char *)tf->x[1];
            size_t count = (size_t)tf->x[2];

            if (fd == 1 || fd == 2) {
                for (size_t i = 0; i < count; i++) uart_putc(buf[i]);
                tf->x[0] = count;
            } else if (fd >= 0 && fd < MAX_FD && current && current->fd_table[fd]) {
                tf->x[0] = vfs_write(current->fd_table[fd], buf, count);
            } else {
                tf->x[0] = (uint64_t)-1;
            }
            break;
        }

        case SYS_READ: {
            int fd = (int)tf->x[0];
            void *buf = (void *)tf->x[1];
            size_t count = (size_t)tf->x[2];

            if (count > 4096 ||
                (count > 0 && !user_range_valid((uint64_t)buf, count))) {
                tf->x[0] = (uint64_t)-1;
                break;
            }

            // Standard Input
            if (fd == 0) {
                char *cbuf = (char *)buf;
                size_t bytes_read = 0;

                while (bytes_read < count) {
                    // Wait until a character arrives from UART.
                    char c = uart_getc();

                    cbuf[bytes_read++] = c;

                    // Stop reading when Enter is received.
                    if (c == '\n') {
                        break;
                    }
                }

                tf->x[0] = bytes_read;
            }

            // Regular File
            else if (fd >= 0 &&
                    fd < MAX_FD &&
                    current &&
                    current->fd_table[fd]) {

                tf->x[0] = vfs_read(
                    current->fd_table[fd],
                    buf,
                    count
                );
            }

            else {
                tf->x[0] = (uint64_t)-1;
            }

            break;
        }

        case SYS_OPEN: {
            const char *path = (const char *)tf->x[0];
            int flags = (int)tf->x[1];

            if (!current) {
                tf->x[0] = (uint64_t)-1;
                break;
            }

            int free_fd = -1;
            for (int i = 3; i < MAX_FD; i++) {
                if (current->fd_table[i] == NULL) {
                    free_fd = i;
                    break;
                }
            }

            if (free_fd < 0) {
                tf->x[0] = (uint64_t)-1;
                break;
            }

            file_t *file = vfs_open(path, flags);
            if (file) {
                current->fd_table[free_fd] = file;
                tf->x[0] = free_fd;
            } else {
                tf->x[0] = (uint64_t)-1;
            }
            break;
        }

        case SYS_CLOSE: {
            int fd = (int)tf->x[0];
            if (fd >= 0 && fd < MAX_FD && current && current->fd_table[fd]) {
                vfs_close(current->fd_table[fd]);
                current->fd_table[fd] = NULL;
                tf->x[0] = 0;
            } else {
                tf->x[0] = (uint64_t)-1;
            }
            break;
        }

        case SYS_GETPID:
            tf->x[0] = current ? (int64_t)current->pid : -1;
            break;

        case SYS_SLEEP:
            sched_sleep((uint64_t)tf->x[0]);
            tf->x[0] = 0;
            break;

        case SYS_EXIT:
            sched_exit_task((int)tf->x[0]);
            break;

        case SYS_READDIR: {
            const char *path = (const char *)tf->x[0];
            vfs_list_dir(path);
            tf->x[0] = 0;
            break;
        }

        case SYS_PS: {
            sched_print_tasks();
            tf->x[0] = 0;
            break;
        }

        case SYS_EXECVE: {
            const char *path = (const char *)tf->x[0];
            file_t *f = vfs_open(path, O_RDONLY);
            if (!f) {
                tf->x[0] = (uint64_t)-1;
                break;
            }

            char *elf_buf = (char *)kmalloc(4096);
            if(!elf_buf){
                vfs_close(f);
                tf->x[0] = (uint64_t)-1;
                break;
            }
            
            int64_t len = vfs_read(f, elf_buf, 4096);
            vfs_close(f);

            if (len <= 0) {
                kfree(elf_buf);
                tf->x[0] = (uint64_t)-1;
                break;
            }

            uint64_t entry = elf_load(elf_buf, (size_t)len);
            kfree(elf_buf);

            if (entry == 0) {
                tf->x[0] = (uint64_t)-1;
                break;
            }

            task_create_user_elf(path, (void (*)(void))entry);
            tf->x[0] = 0;
            break;
        }

        default:
            uart_puts("[SYSCALL ERROR] Unknown Syscall Number: ");
            uart_put_hex(syscall_num);
            uart_puts("\n");
            tf->x[0] = (uint64_t)-1;
            break;
    }
}