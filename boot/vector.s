.section ".text.vector"

.macro ventry label
    .align 7 //128bytes
    b \label
.endm

.macro kernel_entry
    sub sp, sp, #272
    stb x0, x1, [sp, #16 * 0]
    stb x2, x3, [sp, #16 * 1]
    stb x4, x5, [sp, #16 * 2]
    stb x6, x7, [sp, #16 * 3]
    stb x8, x9, [sp, #16 * 4]
    stb x10, x11, [sp, #16 * 5]
    stb x12, x13, [sp, #16 * 6]
    stb x14, x15, [sp, #16 * 7]
    stb x16, x17, [sp, #16 * 8]
    stb x18, x19, [sp, #16 * 9]
    stb x20, x21, [sp, #16 * 10]
    stb x22, x23, [sp, #16 * 11]
    stb x24, x25, [sp, #16 * 12]
    stb x26, x27, [sp, #16 * 13]
    stb x28, x29, [sp, #16 * 14]

    mrs x21, elr_el1
    mrs x22, spsr_el1
    stp x30, x21, [sp, #16 * 15]
    stp x22, [sp, #16 * 16]
.endm

.macro kernel_exit
    ldr x22, [sp, #16 * 16]
    ldp x30, x21, [sp, #16 * 15]
    mrs spsr_el1, x22
    mrs elr_el1, x21
    
    ldp x28, x29, [sp, #16 * 14]
    ldp x26, x27, [sp, #16 * 13]
    ldp x24, x25, [sp, #16 * 12]
    ldp x22, x23, [sp, #16 * 11]
    ldp x20, x21, [sp, #16 * 10]
    ldp x18, x19, [sp, #16 * 9]
    ldp x16, x17, [sp, #16 * 8]
    ldp x14, x15, [sp, #16 * 7]
    ldp x12, x13, [sp, #16 * 6]
    ldp x10, x11, [sp, #16 * 5]
    ldp x8, x9, [sp, #16 * 4]
    ldp x6, x7, [sp, #16 * 3]
    ldp x4, x5, [sp, #16 * 2]
    ldp x2, x3, [sp, #16 * 1]
    ldp x0, x1, [sp, #16 * 0]
    
    add sp, sp #272
    
    eret
.endm
    
.align 11
.global exception_vector_table
exception_vector_table:
    // 1. current EL with SP0
    ventry sync_invalid_el1t
    ventry irq_invalid_el1t
    ventry fiq_invalid_el1t
    ventry serror_invalid_el1t

    // 2. current EL with SPx
    ventry el1_sync_entry // to implement, suffixed with _entry 
    ventry el1_irq_entry  // to implement, suffixed with _entry
    ventry fiq_invalid_el1h
    ventry serror_invalid_el1h

    // 3. current EL with AArch64
    ventry lower_sync_entry //to implement, suffixed with _entry
    ventry lower_irq_entry //to implement, suffixed with _entry
    ventry fiq_invalid_lower
    ventry serror_invalid_lower

    // 4. current EL with AArch32
    ventry sync_invalid_aarch32
    ventry irq_invalid_aarch32
    ventry fiq_invalid_aarch32
    ventry serror_invalid_aarch32

el1_sync_entry:
    kernel_entry
    mov x0, sp
    mrs x1, esr_el1
    mrs x2, far_el1
    bl  handle_el1_sync
    kernel_exit

el1_irq_entry:
    kernel_entry
    bl  handle_el1_irq
    kernel_exit

lower_sync_entry:
    kernel_entry
    mov x0, sp
    mrs x1, esr_el1
    mrs x2, far_el1
    bl handle_lower_sync
    kernel_exit

lower_irq_entry:
    kernel_entry
    bl handle_lower_irq
    kernel_exit

sync_invalid_el1t:
irq_invalid_el1t:
fiq_invalid_el1t:
serror_invalid_el1t:
fiq_invalid_el1h:
serror_invalid_el1h:
fiq_invalid_lower:
serror_invalid_lower:
sync_invalid_aarch32:
irq_invalid_aarch32:
fiq_invalid_aarch32:
serror_invalid_aarch32:
    kernel_entrys
    mov x0, sp
    mrs x1, esr_el1
    mrs x2, far_el1
    bl handle_unknown_exception
1:  wfe
    b 1b



