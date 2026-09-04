#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdint.h>

typedef struct{
    uint64_t x[30]; //240B
    uint64_t lr; //8B
    uint64_t elr_el1; //8B
    uint64_t spsr_el1;//8B
    uint64_t unused; //to packing for 272B 
} trap_frame_t;

void exception_init(void);

static inline void enable_irq(void){
    asm volatile("msr daifclr, #2" ::: "memory");//Bit 1 (I bit) clear -> enable IRQ

}

static inline void disable_irq(void){
    asm volatile("msr daifset, #2" ::: "memory");//Bit 1 (I bit) set  -> disable IRQ
}

void handle_el1_sync(trap_frame_t *tf, uint64_t esr, uint64_t far);
void handle_el1_irq(void);
void handle_lower_sync(trap_frame_t *tf, uint64_t esr, uint64_t far);
void handle_lower_irq(void);
void handle_unknown_exception(trap_frame_t *tf, uint64_t esr, uint64_t far);

#endif