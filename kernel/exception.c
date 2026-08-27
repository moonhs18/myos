#include <exception.h>
#include <uart.h>

extern char exception_vector_table[];

void exception_init(void)
{
    asm volatile("msr vbar_el1, %0" : : "r"(exception_vector_table));
}

static const char* decode_exception_class(uint64_t ec){
    switch(ec){
        case 0x00: return "Unknown Reason";
        case 0x01: return "Trapped WFI/WFE Instruction";
        case 0x0E: return "Illegal  Execution state";
        case 0x15: return "SVC Instruction Execution (Syscall AArch64)";
        case 0x20: return "Instruction Abort (Lower EL)";
        case 0x21: return "Instruction Abort (Current EL)";
        case 0x22: return "PC Alignment Falult";
        case 0x24: return "Data Abort (Lower EL)";
        case 0x25: return "Data Abort (Current EL - Invalid Memory Access)";
        case 0x26: return "SP Alignment Fault";
        default:   return "Reversed / Unhandled Exception Class";
    }
}

static void print_panic_dump(trap_frame_t *tf, uint64_t esr, uint64_t far, const char *title){
    uint64_t ec = (esr >> 26) & 0x3F;

    uart_puts("\n=======================================================\n");
    uart_puts("                 [KERNEL PANIC OCCUREED]               \n");
    uart_puts("=======================================================\n");
    uart_puts("Type                 :"); uart_puts(title); uart_puts("\n");
    uart_puts("Faulting PC (ELR)    :"); uart_put_hex(tf->elr_el1); uart_puts("\n");
    uart_puts("Fault Address (FAR)  :"); uart_put_hex(far); uart_puts("\n");
    uart_puts("Syndrome (ESR)       :"); uart_put_hex(esr); uart_puts("\n");
    uart_puts("Class Description    :"); uart_puts(decode_exception_class(ec)); uart_puts("\n\n");

    uart_puts("--- General Purpose Register (Trap Frame Dump) ----\n");
    for(int i = 0;i < 30;i += 2){
        uart_puts("X");
        if(i < 10) uart_putc('0' + i);
        else {uart_putc('0' + (i/10)); uart_putc('0' + (i%10));}
        uart_puts(": ");
        uart_put_hex(tf->x[i]);

        uart_puts("    X");
        if((i+1) < 10) uart_putc('0' + (i+1));
        else {uart_putc('0' + ((i+1) / 10)); uart_putc('0' + ((i+1) % 10));}
        uart_puts(": ");
        uart_put_hex(tf->x[i+1]);
        uart_puts("\n");
    }
    uart_puts("LR (X30) : "); uart_put_hex(tf->lr);
    uart_puts("   SPSR_EL1: "); uart_put_hex(tf->spsr_el1);
    uart_puts("\n=======================================================\n");
}

void handle_el1_sync(trap_frame_t *tf, uint64_t esr, uint64_t far){
    print_panic_dump(tf, esr, far, "Synchronous Exception inside Kernel (EL1)");
    while (1)
    {
        asm volatile("wfe");
    }
}

void handle_el1_irq(void){
    uart_puts("[IRQ] EL1 Kernel Interrupt Triggered!\n");
}

void handle_lower_sync(trap_frame_t *tf, uint64_t esr, uint64_t far){
    uint64_t ec = (esr >> 26) & 0x3f;
    if(ec == 0x15){
        uart_puts("[SYSCALL] System Call Invoked from User Mode!\n");
        return;
    }


    print_panic_dump(tf, esr, far, "User Space (EL0) Exception\n");
    while(1);
}

void handle_lower_irq(void){
    uart_puts("[IRQ] User Space Interrupt Triggered!\n");
}

void handle_unknown_exception(trap_frame_t *tf, uint64_t esr, uint64_t far){
    print_panic_dump(tf, esr, far, "Unknown / Unexpected Trap Vector");
    while(1);
}
