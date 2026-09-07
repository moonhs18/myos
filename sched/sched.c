#include "sched.h"
#include "pmm.h"
#include "heap.h"
#include "uart.h"

static task_struct_t *current_task = 0;
static task_struct_t *task_list_head = 0;
static uint32_t next_pid = 0;

void sched_init(void){
    //Register the main kernel running immediately after boot as PID 0
    task_struct_t *idle_task = (task_struct_t *)kmalloc(sizeof(task_struct_t));
    if(!idle_task){
        uart_puts("[SCHED ERROR] Failed to allocate Idle Task TCB!\n");
        return;
    }

    idle_task->pid = next_pid++;
    idle_task->state = TASK_RUNNING;
    idle_task->time_slice = DEFAULT_TIME_SLICE;
    idle_task->stack_base = 0;//use main kernel
    idle_task->entry_fn = 0;
    idle_task->next = idle_task;//Single task circular scheduling

    current_task = idle_task;
    task_list_head = idle_task;

    uart_puts("[SCHED] Schedular Initialized (Idle Task PID 0 Activate)\n");
}

task_struct_t *task_create(void (*entry_fn)(void)){
    if(!entry_fn) return 0;

    //Dynamic allocate TCB struct
    task_struct_t *new_task = (task_struct_t*)kmalloc(sizeof(task_struct_t));
    if(!new_task) return 0;

    //Allocation one page(4KB) of physical memmory for the task's private stack
    void *stack = pmm_alloc_page();
    if(!stack){
        kfree(new_task);
        return 0;
    }

    new_task->pid = next_pid++;
    new_task->state = TASK_READY;
    new_task->time_slice = DEFAULT_TIME_SLICE;
    new_task->stack_base = stack;
    new_task->entry_fn = entry_fn;

    //Calculate the stack top
    uint64_t stack_top = ((uint64_t)stack + TASK_STACK_SIZE) & ~0xFULL;

    //Initialize CPU Context
    new_task->context.x19 = (uint64_t)entry_fn;
    new_task->context.lr = (uint64_t)task_entry_trampoline;
    new_task->context.sp = stack_top;
    new_task->context.fp = 0;

    //Insert into circular linked list
    new_task->next = task_list_head->next;
    task_list_head->next = new_task;

    return new_task;
}

void schedule(void){
    if(!current_task) return;

    task_struct_t *prev = current_task;
    task_struct_t *next = current_task->next;

    //Find the next runnable task in the READY state
    while (next != prev)
    {
        if(next->state == TASK_READY || next->state == TASK_RUNNING){
            break;
        }
        next = next->next;
    }

    if(next == prev) return; //No Runnable Task exists

    if(prev->state == TASK_RUNNING){
        prev->state = TASK_READY;
    }
    next->state = TASK_RUNNING;
    current_task = next;

    cpu_switch_to(prev, next);
}

void sched_tick(void){
    if(!current_task) return;

    if(current_task->time_slice > 0){
        current_task->time_slice--;
    }

    if(current_task->time_slice == 0){
        current_task->time_slice = DEFAULT_TIME_SLICE;
        schedule();
    }
}

void task_exit(void){
    current_task ->state = TASK_TERMINATED;
    schedule();

    while(1){
        asm volatile("wfe");
    }
}

task_struct_t *sched_get_current_task(void){
    return current_task;
}