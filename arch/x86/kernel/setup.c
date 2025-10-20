// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "x86_64: " fmt

#include <asm/gdt.h>
#include <idt.h>
#include <seren/init.h>
#include <seren/printk.h>
#include <pic.h>

void arch_init(void) {
	pr_info("Initializing x86_64 architecture...\n");

	pr_info("Initializing GDT...\n");
	gdt_init();

	pr_info("Initializing IDT...\n");
	idt_init();

	pr_info("Initializing and remapping legacy PIC...\n");
	pic_remap_and_init();

	pr_info("x86_64 architecture initialization complete\n");
}

static int __init setup_arch(void) {
	arch_init();

	return 0;
}

arch_initcall(setup_arch);
