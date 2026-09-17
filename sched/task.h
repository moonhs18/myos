#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stddef.h>

#include "memory_layout.h"

#define TASK_STACK_SIZE         4096//Each Task size
#define DEFAULT_TIME_SLICE      5   //Default 50ms


//ARM64 Callee Saved Register
typedef struct cpu_context
{
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;    //x29
    uint64_t lr;    //x39
    uint64_t sp;    //Stack Pointer


    uint64_t user_sp;

}cpu_context_t;

typedef enum task_state{
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
}task_state_t;

//Task Control Block(TCB)
typedef struct task_struct
{
    cpu_context_t context;      //offset 0
    uint32_t pid;
    task_state_t state;
    uint32_t time_slice;        //remain ruuning ticks
    uint32_t sleep_ticks;       //sys_sleep waiting tick

    void *stack_base;           //stack start physical addr
    void *user_stack_base;      //user stack (EL0)
    
    uint64_t user_stack_top;    //user stack VA
    
    void (*entry_fn)(void);     //task 

    uint8_t is_user;            //1 = EL0 Task, 0 = Kernel Task

    struct task_struct *next;   //run queue linked list pointer
}task_struct_t;

#endif