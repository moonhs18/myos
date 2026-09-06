#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stddef.h>

#define TASK_STACK_SIZE     4096//Each Task size
#define DEFAULT_TIME_SLICE  5   //Default 50ms

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
    uint64_t lr;    //x30
    uint64_t sp;    //Stack Pointer
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
    void *stack_base;           //stack start physical addr
    void (*entry_fn)(void);     //task 
    struct task_struct *next;   //run queue linked list pointer
}task_struct_t;

#endif