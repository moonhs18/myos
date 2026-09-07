#ifndef SCHED_H
#define SCHED_H

#include "task.h"

void sched_init(void);
task_struct_t *task_create(void(*entry_fn)(void));
void schedule(void);
void sched_tick(void);
void task_exit(void);
task_struct_t *sched_get_current_task(void);

extern void cpu_switch_to(task_struct_t *prev, task_struct_t*next);
extern void task_entry_trampoline(void);

#endif