// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "init: " fmt

#include <arch.h>
#include <asm/irqflags.h>
#include <lib/string.h>
#include <limine.h>
#include <pic.h>
#include <seren/fs/vfs.h>
#include <seren/init.h>
#include <seren/input.h>
#include <seren/interrupt.h>
#include <seren/mm/pmm.h>
#include <seren/mm/slab.h>
#include <seren/panic.h>
#include <seren/pit.h>
#include <seren/printk.h>
#include <seren/sched/sched.h>
#include <seren/tty.h>
#include <seren/types.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0
};
// clang-format on

static void do_initcalls(void) {
	for (initcall_t *call = &__initcall_start; call < &__initcall_end;
	     call++) {
		int ret = (*call)();
		if (ret != 0) {
			panic("initcall failed with exit code %d", ret);
		}
	}
}

void kmain(void) {
	do_initcalls();

	pr_info("Seren OS is booting...\n");
	pr_info("LFB GFX, PSF Font, console initialized.\n");

	sched_init();

	pr_info("Initialization sequence complete. You can now type. See you "
		"<3\n");

	for (;;) {
		__asm__ volatile("hlt");
	}
}
