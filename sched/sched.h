#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include <stddef.h>

#include "task.h"

void sched_init(uint64_t initial_sp);
task_struct_t *task_create(void(*entry_fn)(void));
task_struct_t *task_create_user(void(*entry_fn)(void));

void schedule(void);
void sched_tick(void);
void task_exit(void);

void sched_yield(void);
void sched_sleep(uint64_t ticks);
void sched_exit_task(int code);


task_struct_t *sched_get_current_task(void);
uint32_t sched_get_live_user_tasks(void);

extern void cpu_switch_to(task_struct_t *prev, task_struct_t*next);
extern void task_entry_trampoline(void);
extern void user_first_return(void);

#endif