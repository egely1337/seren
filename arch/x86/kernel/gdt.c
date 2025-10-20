// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#define pr_fmt(fmt) "gdt: " fmt

#include <asm/gdt.h>
#include <lib/string.h>
#include <seren/printk.h>

/* Assembly helpers to lead the GDT and TSS registers. */
extern void gdt_flush(struct gdt_ptr *gdtp);
extern void tss_flush(u16 selector);

/** The TSS for this CPU. */
static struct tss tss __attribute__((aligned(16)));

/** A stack for handling double faults, set up via the TSS and IST. */
static u8 ist_double_fault_stack[IST_STACK_SIZE] __attribute__((aligned(16)));

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtp;

/*
 * gdt_set_entry - Configure a standard GDT descriptor.
 * @num: The index in the GDT array.
 * @access: The access byte.
 * @flags: Flags that define granularity and long mode.
 *
 * This sets up a flat 64-bit segment where the base is 0 and the limit
 * covers the entire address space. In long mode segmentation is mostly
 * disabled but we still need these descriptors to define privilege levels.
 */
static void gdt_set_entry(int num, u8 access, u8 flags) {
	gdt[num].limit0 = 0xFFFF;
	gdt[num].base0 = 0;
	gdt[num].base1 = 0;
	gdt[num].access_byte = access;
	/**
	 * The lower 4 bits of limit1_flags are the upper 4 bits of the limit.
	 * Combining with limit0 gives a 20-bit limit of 0xFFFFF. When the
	 * 4K granularity flag is set, this covers the full 4GB space (or 64-bit
	 * space in long mode.)
	 */
	gdt[num].limit1_flags = 0x0F | flags;
	gdt[num].base2 = 0;
}

/*
 * tss_set_entry - Configure the special 16-byte TSS descriptor.
 * @num: The index in the GDT array where the TSS descriptor starts.
 * @base: The 64-bit linear address of our `tss` structure.
 */
static void tss_set_entry(int num, u64 base) {
	struct tss_entry *desc = (struct tss_entry *)&gdt[num];

	desc->limit0 = sizeof(tss) - 1;
	desc->base0 = base & 0xFFFF;
	desc->base1 = (base >> 16) & 0xFF;
	desc->base2 = (base >> 24) & 0xFF;
	desc->base3 = (base >> 32) & 0xFFFFFFFF;
	desc->access_byte = GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
			    GDT_ACCESS_TYPE_SYS | GDT_ACCESS_TYPE_TSS_AVAIL;
	desc->limit1_flags = 0;
	desc->reserved = 0;
}

/**
 * gdt_init - Set up the GDT and TSS for 64-bit operation.
 */
void gdt_init(void) {
	/**
	 * First, set up the TSS. We only really care about the IST for now.
	 * IST[0] corresponds to index 1, so we set that to the top of our
	 * special double fault stack.
	 */
	memset(&tss, 0, sizeof(tss));
	tss.ist[IST_DOUBLE_FAULT - 1] =
	    (u64)ist_double_fault_stack + IST_STACK_SIZE;

	/** Now, populate the GDT entries. The selectors are byte offsets, so we
	 * divide by 8 to get the array index. */

	/** Kernel Code Segment (Ring 0) */
	gdt_set_entry(GDT_KERNEL_CODE_SELECTOR / 8,
		      GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
			  GDT_ACCESS_TYPE_CODE_DATA |
			  GDT_ACCESS_TYPE_CODE_READ | GDT_ACCESS_TYPE_CODE_EXEC,
		      GDT_FLAG_GRAN_4K | GDT_FLAG_64BIT);

	/** Kernel Data Segment (Ring 0) */
	gdt_set_entry(
	    GDT_KERNEL_DATA_SELECTOR / 8,
	    GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_TYPE_CODE_DATA |
		GDT_ACCESS_TYPE_DATA_WRITE,
	    GDT_FLAG_GRAN_4K | GDT_FLAG_32BIT); /* Data segs use D/B=1 */

	/** User Code Segment (Ring 3) */
	gdt_set_entry(GDT_USER_CODE_SELECTOR / 8,
		      GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
			  GDT_ACCESS_TYPE_CODE_DATA |
			  GDT_ACCESS_TYPE_CODE_READ | GDT_ACCESS_TYPE_CODE_EXEC,
		      GDT_FLAG_GRAN_4K | GDT_FLAG_64BIT);

	/** User Data Segment (Ring 3) */
	gdt_set_entry(GDT_USER_DATA_SELECTOR / 8,
		      GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
			  GDT_ACCESS_TYPE_CODE_DATA |
			  GDT_ACCESS_TYPE_DATA_WRITE,
		      GDT_FLAG_GRAN_4K | GDT_FLAG_32BIT);

	/** TSS Segment */
	tss_set_entry(GDT_TSS_SELECTOR / 8, (u64)&tss);

	gdtp.limit = sizeof(gdt) - 1;
	gdtp.base = (u64)&gdt;

	gdt_flush(&gdtp);
	tss_flush(GDT_TSS_SELECTOR);

	pr_info("GDT and TSS initialized and loaded\n");
}