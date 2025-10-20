// SPDX-License-Identifier: Apache-2.0

#ifndef _ASM_X86_64_GDT_H
#define _ASM_X86_64_GDT_H

#include <seren/mm/pmm.h>
#include <seren/types.h>

/**
 * Segment selectors for IDT.
 * These are offsets into the GDT, which will be loaded into segment registers
 * like %cs, %ds, etc.
 */
#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_USER_CODE_SELECTOR	 0x18
#define GDT_USER_DATA_SELECTOR	 0x20
#define GDT_TSS_SELECTOR	 0x28

/**
 * GDT Layout
 * 0: Null Descriptor
 * 1: Kernel Code (Ring 0)
 * 2: Kernel Data (Ring 0)
 * 3: User Code (Ring 3)
 * 4: User Data (Ring 3)
 * 5, 6: Task State Segment (TSS) - occupies two slots because its 16 bytes.
 */
#define GDT_ENTRIES 7

#define IST_DOUBLE_FAULT 1
#define IST_STACK_SIZE	 PAGE_SIZE

#define GDT_ACCESS_PRESENT	  (1 << 7) /* Present */
#define GDT_ACCESS_RING0	  (0 << 5) /* Descriptor Privilege Level 0 */
#define GDT_ACCESS_RING3	  (3 << 5) /* Descriptor Privilege Level 3 */
#define GDT_ACCESS_TYPE_SYS	  (0 << 4) /* System segment */
#define GDT_ACCESS_TYPE_CODE_DATA (1 << 4) /* Code or Data segment */

#define GDT_ACCESS_TYPE_CODE_EXEC  (1 << 3) /* Executable */
#define GDT_ACCESS_TYPE_DATA_WRITE (1 << 1) /* Writable */
#define GDT_ACCESS_TYPE_CODE_READ  (1 << 1) /* Readable */

#define GDT_ACCESS_TYPE_TSS_AVAIL 0x9

#define GDT_FLAG_64BIT	 (1 << 5) /* Long mode (64-bit) */
#define GDT_FLAG_32BIT	 (1 << 6) /* 32-bit segment */
#define GDT_FLAG_GRAN_4K (1 << 7) /* Granularity (limit is in 4K units) */

/**
 * struct gdt_entry - Standard 8-byte GDT descriptor
 */
struct gdt_entry {
	u16 limit0;
	u16 base0;
	u8 base1;
	u8 access_byte;
	u8 limit1_flags;
	u8 base2;
} __attribute__((packed));

/**
 * struct tss_entry - 16-byte GDT descriptor for the TSS
 */
struct tss_entry {
	u16 limit0;
	u16 base0;
	u8 base1;
	u8 access_byte;
	u8 limit1_flags;
	u8 base2;
	u32 base3;
	u32 reserved;
} __attribute__((packed));

/**
 * struct gdt_ptr - Pointer structure for the LGDT instruction
 *
 * This is the format the CPU expects for the `lgdt` instruction.
 */
struct gdt_ptr {
	u16 limit;
	u64 base;
} __attribute__((packed));

/**
 * struct tss - Task State Segment
 *
 * The TSS is used in 64-bit mode primarily to define stack pointers for
 * privilege changes and for IST.
 */
struct tss {
	u32 reserved0;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u64 reserved1;
	u64 ist[7];
	u64 reserved2;
	u16 reserved3;
	u16 iopb_offset;
} __attribute__((packed));

/**
 * gdt_init - Initialize and load the GDT and TSS.
 */
void gdt_init(void);

#endif /* _ASM_X86_64_GDT_H */