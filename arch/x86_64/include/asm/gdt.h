#ifndef _ASM_X86_64_GDT_H
#define _ASM_X86_64_GDT_H

#include <nucleus/mm/pmm.h>
#include <nucleus/types.h>

#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_USER_CODE_SELECTOR   0x18
#define GDT_USER_DATA_SELECTOR   0x20
#define GDT_TSS_SELECTOR         0x28

/**
 * 0: null
 * 1: kernel code
 * 2: kernel data
 * 3: user code
 * 4: user data
 * 5, 6: TSS
 */
#define GDT_ENTRIES 7

#define IST_DOUBLE_FAULT 1
#define IST_STACK_SIZE   PAGE_SIZE

#define GDT_ACCESS_PRESENT        (1 << 7) /* P */
#define GDT_ACCESS_RING0          (0 << 5) /* DPL */
#define GDT_ACCESS_RING3          (3 << 5) /* DPL */
#define GDT_ACCESS_TYPE_SYS       (0 << 4) /* S (System) */
#define GDT_ACCESS_TYPE_CODE_DATA (1 << 4) /* S (Code/Data) */

#define GDT_ACCESS_TYPE_CODE_EXEC  (1 << 3) /* E */
#define GDT_ACCESS_TYPE_DATA_WRITE (1 << 1) /* W */
#define GDT_ACCESS_TYPE_CODE_READ  (1 << 1) /* R */

#define GDT_ACCESS_TYPE_TSS_AVAIL 0x9

#define GDT_FLAG_64BIT   (1 << 5) /* L */
#define GDT_FLAG_32BIT   (1 << 6) /* D/B */
#define GDT_FLAG_GRAN_4K (1 << 7) /* G */

struct gdt_entry {
    u16 limit0;
    u16 base0;
    u8 base1;
    u8 access_byte;
    u8 limit1_flags;
    u8 base2;
} __attribute__((packed));

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

struct gdt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed));

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

void gdt_init(void);

#endif /* _ASM_X86_64_GDT_H */