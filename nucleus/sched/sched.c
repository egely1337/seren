// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "sched: " fmt

#include <asm/gdt.h>
#include <lib/string.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/sched/sched.h>

static task_t g_task_table[MAX_TASKS];
static volatile pid_t g_current_task_id = 0;
static volatile pid_t g_highest_pid = 0;

static void task_exit(void) {
	pr_debug("task %u ('%s') is exiting\n", g_current_task_id,
		 g_task_table[g_current_task_id].name);

	local_irq_disable();
	g_task_table[g_current_task_id].state = TASK_STATE_DEAD;
	local_irq_enable();

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

	pr_info("initialized; idle task created with PID %u\n",
		g_current_task_id);
}

pid_t create_task(const char *name, void (*entry_point)(void)) {
	u64 flags = local_irq_save();

	pid_t new_pid = -1;
	// TODO: Implement task slot reuse by searching for TASK_STATE_DEAD
	if (g_highest_pid >= MAX_TASKS) {
		pr_err("failed to create task '%s': max tasks reached\n", name);
		local_irq_restore(flags);
		return -1;
	}

	new_pid = g_highest_pid++;

	task_t *new_task = &g_task_table[new_pid];
	new_task->id = new_pid;
	new_task->name = name;

	struct page *stack_page = alloc_page();
	if (!stack_page) {
		pr_err("failed to create task '%s': out of physical memory\n",
		       name);
		g_highest_pid--;
		local_irq_restore(flags);
		return -1;
	}

	void *stack = page_to_virt(stack_page);

	new_task->stack_base = (uintptr_t)stack;
	uintptr_t stack_top = new_task->stack_base + PAGE_SIZE;

	struct pt_regs *context =
	    (struct pt_regs *)(stack_top - sizeof(struct pt_regs));
	memset(context, 0, sizeof(struct pt_regs));

	uintptr_t task_entry_stack_top = (uintptr_t)context;
	task_entry_stack_top -= 8;
	*((u64 *)task_entry_stack_top) = (u64)task_exit;

	context->rip = (u64)entry_point;
	context->cs = GDT_KERNEL_CODE_SELECTOR;
	context->rflags = 0x202;
	context->rsp = task_entry_stack_top;
	context->ss = GDT_KERNEL_DATA_SELECTOR;

	new_task->stack_pointer = (uintptr_t)context;
	new_task->state = TASK_STATE_READY;

	pr_info("created task '%s' with PID %u\n", name, new_pid);

	local_irq_restore(flags);
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

		if (next_task_id == 0)
			continue;

		if (g_task_table[next_task_id].state == TASK_STATE_READY) {
			g_current_task_id = next_task_id;
			g_task_table[g_current_task_id].state =
			    TASK_STATE_RUNNING;

			/*pr_debug("switching to task %u ('%s')\n",
				 g_current_task_id,
				 g_task_table[g_current_task_id].name);*/
			// FUCKS UP THE CONSOLE SO I COMMENTED IT, BUT IT WORKS.
			return g_task_table[g_current_task_id].stack_pointer;
		}
	}

	// If we get here it means no other task was ready.
	// The current task (which we just set to READY) is the only option, so
	// we continue running it. This happens frequently in the idle loop.
	g_current_task_id = 0;
	g_task_table[g_current_task_id].state = TASK_STATE_RUNNING;
	return g_task_table[g_current_task_id].stack_pointer;
}