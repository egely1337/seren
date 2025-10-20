// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_SCHED_H
#define _SEREN_SCHED_H

#include <seren/types.h>

#define KERNEL_TASK_NAME "kernel_idle"
#define MAX_TASKS	 1337

typedef s32 pid_t;

typedef enum {
	TASK_STATE_DEAD = 0,
	TASK_STATE_RUNNING,
	TASK_STATE_READY,
	TASK_STATE_BLOCKED,
} task_state_t;

typedef struct task {
	pid_t id;
	task_state_t state;
	const char *name;

	uintptr_t stack_ptr;

	uintptr_t stack_base;
} task_t;

/**
 * sched_init - Initializes scheduling
 */
void sched_init(void);

/**
 * create_task - Creates a new kernel task.
 */
pid_t create_task(const char *name, void (*entry_point)(void));

/**
 * schedule - The main scheduler function, called by the timer interrupt.
 */
uintptr_t schedule(uintptr_t);

#endif // _SEREN_SCHED_H