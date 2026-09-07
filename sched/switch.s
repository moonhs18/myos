.section ".text"
.global cpu_switch_to
.global task_entry_trampoline

// void cpu_switch_to(task_struct_t *prev, task_struct_t *next);
// x0, prev task(context offset: 0) 
// x1 = nexts task(context offset: 0)

cpu_switch_to:
    //prev task callee register backup and store
    mov x2, sp
    stp x19,x20, [x0, #16 * 0]
    stp x21,x22, [x0, #16 * 1]
    stp x23,x24, [x0, #16 * 2]
    stp x25,x26, [x0, #16 * 3]
    stp x27,x28, [x0, #16 * 4]
    stp x29,x30, [x0, #16 * 5]
    str x2,      [x0, #16 * 6]
    
    //next task callee register and restore
    ldp x19,x20, [x1, #16 * 0]
    ldp x21,x22, [x1, #16 * 1]
    ldp x23,x24, [x1, #16 * 2]
    ldp x25,x26, [x1, #16 * 3]
    ldp x27,x28, [x1, #16 * 4]
    ldp x29,x30, [x1, #16 * 5]
    ldr x2,      [x1, #16 * 6]
    mov sp, x2

    ret
    
//New task entry
task_entry_trampoline:
    //IRQ interrupt enable
    msr daifclr, #2
    //Task function call(in x19)
    blr x19
    //Function exit, task clean routine
    bl task_exit