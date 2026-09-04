#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t target_hz);
void timer_handle_irq(void);
uint64_t timer_get_ticks(void);
void timer_sleep_ticks(uint64_t count);



#endif