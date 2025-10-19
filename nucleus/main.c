// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "init: " fmt

#include <arch.h>
#include <asm/irqflags.h>
#include <drivers/input/keyboard.h>
#include <drivers/pit.h>
#include <lib/string.h>
#include <limine.h>
#include <nucleus/fs/vfs.h>
#include <nucleus/init.h>
#include <nucleus/interrupt.h>
#include <nucleus/mm/pmm.h>
#include <nucleus/mm/slab.h>
#include <nucleus/panic.h>
#include <nucleus/printk.h>
#include <nucleus/sched/sched.h>
#include <nucleus/tty/console.h>
#include <nucleus/types.h>
#include <pic.h>

__attribute__((
    used,
    section(".limine_requests"))) volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used,
	       section(".limine_requests"))) volatile struct limine_hhdm_request
    hhdm_request = {.id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__((
    used, section(".limine_requests"))) volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

static void do_initcalls(void) {
	pr_info("Running initcalls...\n");
	for (initcall_t *call = &__initcall_start; call < &__initcall_end;
	     call++) {
		int ret = (*call)();
		if (ret != 0) {
			panic("initcall failed with exit code %d", ret);
		}
	}
}

void kmain(void) {
	console_init();

	do_initcalls();

	pr_info("Seren OS is booting...\n");
	pr_info("LFB GFX, PSF Font, console initialized.\n");

	sched_init();
	local_irq_enable();

	pr_info(
	    "Initialization sequence complete. You can now type. See you <3\n");

#ifdef SERENOS_TEST_BUILD
	pr_notice("This is the test build!\n");
#endif

	while (1) {
		char c = keyboard_getchar();
		printk("%c", c);
	}
}
