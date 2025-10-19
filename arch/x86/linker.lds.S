/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

/* Alright, this is the blueprint for our Nucleus Kernel's memory layout.
   We're making a 64-bit ELF executable that's gonna live in the "higher-half" of memory.
   This script tells the linker where to put all the pieces, like code and data,
   and makes sure Limine can boot it up nicely. */

/* TODO: Add KASLR support */

#include <asm-generic/linker.lds.h>

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

KERNEL_BASE = 0xffffffff80000000;
KERNEL_PHYS_LOAD_ADDRESS = 0x100000;

PAGE_SIZE = 4K;

PHDRS
{
	phdr_limine_requests	PT_LOAD FLAGS(4);
	phdr_text		PT_LOAD FLAGS(5);
	phdr_rodata		PT_LOAD FLAGS(4);
	phdr_data		PT_LOAD FLAGS(6);

	phdr_init		PT_LOAD FLAGS(5);
}

SECTIONS
{
	. = KERNEL_BASE;

	.limine_requests ALIGN(PAGE_SIZE) : AT(KERNEL_PHYS_LOAD_ADDRESS) {
		KEEP(*(.limine_requests))
	} :phdr_limine_requests

	. = ALIGN(PAGE_SIZE);

	.text : {
		*(.text .text.*)
	} :phdr_text

	. = ALIGN(PAGE_SIZE);

	.rodata : {
		*(.rodata .rodata.*)
	} :phdr_rodata

	. = ALIGN(PAGE_SIZE);

	.data : {
		*(.data .data.*)
	} :phdr_data

	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	} :phdr_data

	. = ALIGN(PAGE_SIZE);

	__init_begin = .;

	.init : {
		. = ALIGN(4);
		*(.init.text)

		. = ALIGN(8);
		*(.init.rodata)
		__initcall_start = .;
		INIT_CALLS
		__initcall_end = .;

		. = ALIGN(4);
		*(.init.data)
	} :phdr_init

	. = ALIGN(PAGE_SIZE);
	__init_end = .;

	_kernel_end = .;

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
		*(.comment)
	}
}