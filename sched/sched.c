#include "sched.h"
#include "pmm.h"
#include "heap.h"
#include "uart.h"
#include "mmu.h"
#include "exception.h"

static task_struct_t *current_task = 0;
static task_struct_t *task_list_head = 0;

static uint32_t next_pid = 0;
static uint32_t user_stack_slot = 0;

void sched_init(uint64_t initial_sp){
    //Register the main kernel running immediately after boot as PID 0
    task_struct_t *idle_task = (task_struct_t *)kmalloc(sizeof(task_struct_t));
    if(!idle_task){
        uart_puts("[SCHED ERROR] Failed to allocate Idle Task TCB!\n");
        return;
    }

    idle_task->pid = next_pid++;
    idle_task->state = TASK_RUNNING;
    idle_task->time_slice = DEFAULT_TIME_SLICE;
    idle_task->sleep_ticks = 0;

    idle_task->stack_base = 0;//use main kernel
    idle_task->user_stack_base = 0;
    idle_task->user_stack_top = 0;

    idle_task->entry_fn = 0;
    
    idle_task->is_user = 0;
    
    idle_task->context.x19 = 0;
    idle_task->context.x20 = 0;
    idle_task->context.x21 = 0;
    idle_task->context.x22 = 0;
    idle_task->context.x23 = 0;
    idle_task->context.x24 = 0;
    idle_task->context.x25 = 0;
    idle_task->context.x26 = 0;
    idle_task->context.x27 = 0;
    idle_task->context.fp = 0;
    idle_task->context.lr = 0;
    idle_task->context.sp = initial_sp;
    idle_task->context.user_sp = 0;
    
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
    new_task->sleep_ticks = 0;

    new_task->stack_base = stack;
    new_task->user_stack_base = 0;
    new_task->user_stack_top = 0;
    
    new_task->entry_fn = entry_fn;
    
    new_task->is_user = 0;

    //Calculate the stack top
    uint64_t stack_top = ((uint64_t)stack + TASK_STACK_SIZE) & ~0xFULL;

    //Initialize CPU Context
    new_task->context.x19 = (uint64_t)entry_fn;
    new_task->context.x20 = 0;
    
    new_task->context.lr = (uint64_t)task_entry_trampoline;
    new_task->context.sp = stack_top;

    new_task->context.user_sp = 0;

    new_task->context.fp = 0;

    //Insert into circular linked list
    new_task->next = task_list_head->next;
    task_list_head->next = new_task;

    return new_task;
}


task_struct_t *task_create_user(void (*entry_fn)(void)){
    if(!entry_fn) return 0;

    uint64_t entry = (uint64_t)entry_fn;

    if(entry < USER_START || entry >= USER_END){
        uart_puts("[SCHED ERROR] User entry is outside user address space!\n");
        return 0;
    }

    task_struct_t *new_task = (task_struct_t*)kmalloc(sizeof(task_struct_t));
    if(!new_task) return 0;

    //Allocation Kernel page(4KB) and User page(4KB) of physical memmory
    void *kstack = pmm_alloc_page();
    void *ustack = pmm_alloc_page();
    if(!kstack || !ustack){
        if(kstack)pmm_free_page(kstack);
        if(ustack)pmm_free_page(ustack);
        
        kfree(new_task);
        return 0;
    }

    uint64_t user_stack_va = USER_STACK_TOP - ((user_stack_slot + 1) * PAGE_SIZE);
    uint64_t user_stack_top = user_stack_va + PAGE_SIZE;

    user_stack_slot++;
    


    uint64_t *root = mmu_get_root_table();

    mmu_map_page(root, user_stack_va, (uint64_t)ustack, MMU_FLAG_USER_DATA);

    mmu_tlb_flush();


    uint64_t kstack_top = ((uint64_t)kstack + TASK_STACK_SIZE) & ~0xFULL;
   
    trap_frame_t *tf = (trap_frame_t *)(kstack_top - sizeof(trap_frame_t));

    uint64_t *raw = (uint64_t *)tf;

    for(size_t i=0; i < sizeof(trap_frame_t)/sizeof(uint64_t); i++){
        raw[i]=0;
    }

    //Initial ELR
    tf->elr_el1 = entry;

    tf->spsr_el1 = 0;


    new_task->pid = next_pid++;
    new_task->state = TASK_READY;
    new_task->time_slice = DEFAULT_TIME_SLICE;
    new_task->sleep_ticks = 0;
    new_task->stack_base = kstack;
    new_task->user_stack_base = ustack;
    new_task->user_stack_top = user_stack_top;
    new_task->entry_fn = entry_fn;
    new_task->is_user = 1;


    //Initialize CPU Context
    new_task->context.x19 = 0;
    new_task->context.x20 = 0;
    new_task->context.x21 = 0;
    new_task->context.x22 = 0;
    new_task->context.x23 = 0;
    new_task->context.x24 = 0;
    new_task->context.x25 = 0;
    new_task->context.x26 = 0;
    new_task->context.x27 = 0;
    new_task->context.fp = 0;

    new_task->context.lr = (uint64_t)user_first_return;
    new_task->context.sp = (uint64_t)tf;
    new_task->context.user_sp = user_stack_top;

    new_task->next = task_list_head->next;
    task_list_head->next = new_task;


    uart_puts("[USER] User task created\n ");
    

    uart_puts("[USER] entry = ");
    uart_put_hex(entry);
    uart_puts("\n");

    uart_puts("[USER] kstack = ");
    uart_put_hex((uint64_t)kstack);
    uart_puts("\n");

    uart_puts("[USER] ustack PA  = ");
    uart_put_hex((uint64_t)ustack);
    uart_puts("\n");

    uart_puts("[USER] ustack VA  = ");
    uart_put_hex((uint64_t)ustack);
    uart_puts("\n");


    uart_puts("[USER] ustack top  = ");
    uart_put_hex(user_stack_top);
    uart_puts("\n");

    return new_task;
}

void schedule(void){
    if(!current_task || !task_list_head) return;

    task_struct_t *prev = current_task;
    task_struct_t *next = prev->next;

    //Find the next runnable task in the READY state
    do
    {
        if(next->state == TASK_READY || next->state == TASK_RUNNING){
            break;
        }
        next = next->next;
    }while (next != prev);

    if(next == prev){
        if(prev->state == TASK_RUNNING)
            return;
        return;
    } 


    if(prev->state == TASK_RUNNING){
        prev->state = TASK_READY;
    }
    next->state = TASK_RUNNING;
    next->time_slice = DEFAULT_TIME_SLICE;

    current_task = next;

    cpu_switch_to(prev, next);
}

void sched_tick(void){
    if(!current_task || !task_list_head) return;

    task_struct_t *task = task_list_head;

    do{
        if(task->state == TASK_BLOCKED && task->sleep_ticks > 0){
            task->sleep_ticks--;
            if(task->sleep_ticks == 0)
                task->state = TASK_READY;
        }
        
        task = task->next;
    }while (task!=task_list_head);
    

    if(current_task->state != TASK_RUNNING){
        schedule();
        return;
    }
    
    if(current_task->time_slice > 0)
        current_task->time_slice--;

    if(current_task->time_slice == 0){
        schedule();
    }
}

void sched_yield(void) {
    schedule();
}

void sched_sleep(uint64_t ticks) {
    task_struct_t *curr = sched_get_current_task();
    if (curr) {
        curr->sleep_ticks = ticks;
        curr->state = TASK_BLOCKED;
        schedule();
    }
}

void sched_exit_task(int code) {
    (void)code;
    task_struct_t *curr = sched_get_current_task();
    if (curr) {
        curr->state = TASK_TERMINATED;
        schedule();
    }
}

void task_exit(void){
    task_struct_t *exiting = current_task;

    if(!exiting)
        return;

    exiting ->state = TASK_TERMINATED;
    exiting ->sleep_ticks = 0;
    exiting ->time_slice = 0;

    schedule();

    asm volatile("msr daifset, #2" ::: "memory");

    while(1){
        asm volatile("wfe");
    }
}

task_struct_t *sched_get_current_task(void){
    return current_task;
}

uint32_t sched_get_live_user_tasks(void){
    if(!task_list_head)
        return 0;

    uint32_t count = 0;
    task_struct_t *curr = task_list_head;

    do{
        if(curr->is_user && curr->state != TASK_TERMINATED)
            count++;
        curr = curr->next;            
    }while(curr != task_list_head);

    return count;
}