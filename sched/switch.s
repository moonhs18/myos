.section ".text"
.global cpu_switch_to
.global task_entry_trampoline
.global user_first_return

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

    //user_sp offset = 104
    ldr x3, [x1, #104]

    msr sp_el0, x3
    
    isb

    ret
    
task_entry_trampoline:

    msr daifclr, #2
    isb
    //Task function call(in x19)
    blr x19
    //Function exit, task clean routine
    bl task_exit

1:
    wfe
    b 1b

user_first_return:
    ldr x22, [sp, #248]
    ldr x21, [sp, #256]

    msr elr_el1, x22
    msr spsr_el1, x21

    ldp x0,x1, [sp, #16 * 0]
    ldp x2,x3, [sp, #16 * 1]
    ldp x4,x5, [sp, #16 * 2]
    ldp x6,x7, [sp, #16 * 3]
    ldp x8,x9, [sp, #16 * 4]
    ldp x10,x11, [sp, #16 * 5]
    ldp x12,x13, [sp, #16 * 6]
    ldp x14,x15, [sp, #16 * 7]
    ldp x16,x17, [sp, #16 * 8]
    ldp x18,x19, [sp, #16 * 9]
    ldp x20,x21, [sp, #16 * 10]
    ldp x22,x23, [sp, #16 * 11]
    ldp x24,x25, [sp, #16 * 12]
    ldp x26,x27, [sp, #16 * 13]
    ldp x28,x29, [sp, #16 * 14]

    ldr x30, [sp, #16 * 15]
    add sp, sp, #272


    eret