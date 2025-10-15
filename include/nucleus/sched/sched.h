#ifndef SCHED_H
#define SCHED_H

#include <nucleus/types.h>

#define KERNEL_TASK_NAME (const char*)"kernel_idle";
#define MAX_TASKS (uint64_t)1337

typedef struct process {
    uint8_t priority;
    uintptr_t stack;
    uint64_t burst_time;
} process_t;
typedef uint64_t pid_t;

typedef struct stack_context {
    /* Callee saved registers*/
	uint32_t rbp, rdi, rsi, rbx;
    /* Popped by ret in yield */
	uint32_t switch_addr;
    /* Popped by us in new kernel stack function. */
	uint32_t data_selector;
    /* Popped by iret */
	uint32_t rip, cs, eflags, usermode_esp, usermode_ss;
} stack_context_t;

/**
 * @brief Gives control to another thread immediately.
 */
void yield(void);

/**
 * @brief Initializes scheduling
 */
void sched_init(void);

/**
 * @brief Create thread function
 * This function will create a process (thread) and will give a name, give a priority.
 * When scheduler bursts yield to created task, CPU will start to executing from <eip> parameter.
 * 
 * @param task_name Identify task name of the created task.
 * @param eip Start address of the created task.
 * @param priority Priority of the task
 */
void create_task_from_address(const char* task_name, uint64_t eip, uint8_t priority);

#endif