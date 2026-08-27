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
void handler_el1_sync(trap_frame_t *tf, uint64_t esr, uint64_t far);
void handler_el1_irq(void);
void handler_lower_sync(trap_frame_t *tf, uint64_t esr, uint64_t far);
void handler_lower_irq(void);
void handler_unknown_exception(trap_frame_t *tf, uint64_t esr, uint64_t far);

#endif