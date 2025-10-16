#include <lib/string.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/sched/sched.h>

#define SCHED_PFX "sched: "

#define sched_info(fmt, ...) pr_info(SCHED_PFX fmt, ##__VA_ARGS__)
#define sched_dbg(fmt, ...)  pr_debug(SCHED_PFX fmt, ##__VA_ARGS__)
#define sched_err(fmt, ...)  pr_err(SCHED_PFX fmt, ##__VA_ARGS__)

static task_t g_task_table[MAX_TASKS];
static volatile pid_t g_current_task_id = 0;
static volatile pid_t g_highest_pid = 0;

#define KERNEL_CODE_SEGMENT 0x28
#define KERNEL_DATA_SEGMENT 0x30

static void task_exit(void) {
    sched_dbg("task %u ('%s') is exiting\n", g_current_task_id,
              g_task_table[g_current_task_id].name);

    interrupts_disable();
    g_task_table[g_current_task_id].state = TASK_STATE_DEAD;

    while (1) {
        __asm__ volatile("hlt");
    }
}

void sched_init(void) {
    memset(g_task_table, 0, sizeof(g_task_table));

    pid_t idle_pid = g_highest_pid++;
    task_t *idle_task = &g_task_table[idle_pid];

    idle_task->id = idle_pid;
    idle_task->name = KERNEL_TASK_NAME;
    idle_task->state = TASK_STATE_RUNNING;
    g_current_task_id = idle_pid;

    sched_info("initialized; idle task created with PID %u\n",
               g_current_task_id);
}

pid_t create_task(const char *name, void (*entry_point)(void)) {
    u64 flags = interrupt_save_and_disable();

    pid_t new_pid = -1;
    // TODO: Implement task slot reuse by searching for TASK_STATE_DEAD
    if (g_highest_pid >= MAX_TASKS) {
        sched_err("failed to create task '%s': max tasks reached\n", name);
        interrupt_restore(flags);
        return -1;
    }

    new_pid = g_highest_pid++;

    task_t *new_task = &g_task_table[new_pid];
    new_task->id = new_pid;
    new_task->name = name;

    void *stack = pmm_alloc_page();
    if (!stack) {
        sched_err("failed to create task '%s': out of physical memory\n", name);
        g_highest_pid--;
        interrupt_restore(flags);
        return -1;
    }

    new_task->stack_base = (uintptr_t)stack;
    uintptr_t stack_top = new_task->stack_base + PAGE_SIZE;

    irq_context_t *context =
        (irq_context_t *)(stack_top - sizeof(irq_context_t));
    memset(context, 0, sizeof(irq_context_t));

    uintptr_t task_entry_stack_top = (uintptr_t)context;
    task_entry_stack_top -= 8;
    *((u64 *)task_entry_stack_top) = (u64)task_exit;

    context->rip_cpu = (u64)entry_point;
    context->cs_cpu = KERNEL_CODE_SEGMENT;
    context->rflags_cpu = 0x202;
    context->rsp_cpu = task_entry_stack_top;
    context->ss_cpu = KERNEL_DATA_SEGMENT;

    new_task->stack_pointer = (uintptr_t)context;
    new_task->state = TASK_STATE_READY;

    sched_info("created task '%s' with PID %u\n", name, new_pid);

    interrupt_restore(flags);
    return new_pid;
}

uintptr_t schedule(uintptr_t current_stack_pointer) {
    g_task_table[g_current_task_id].stack_pointer = current_stack_pointer;

    if (g_task_table[g_current_task_id].state == TASK_STATE_RUNNING) {
        g_task_table[g_current_task_id].state = TASK_STATE_READY;
    }

    pid_t next_task_id = g_current_task_id;
    for (int i = 0; i < g_highest_pid; i++) {
        next_task_id = (next_task_id + 1) % g_highest_pid;
        if (g_task_table[next_task_id].state == TASK_STATE_READY) {
            g_current_task_id = next_task_id;
            g_task_table[g_current_task_id].state = TASK_STATE_RUNNING;

            /*sched_dbg("switching to task %u ('%s')\n", g_current_task_id,
                      g_task_table[g_current_task_id].name);*/
            // FUCKS UP THE CONSOLE SO I COMMENTED IT, BUT IT WORKS.
            return g_task_table[g_current_task_id].stack_pointer;
        }
    }

    // If we get here it means no other task was ready.
    // The current task (which we just set to READY) is the only option, so we
    // continue running it. This happens frequently in the idle loop.
    g_task_table[g_current_task_id].state = TASK_STATE_RUNNING;
    return g_task_table[g_current_task_id].stack_pointer;
}