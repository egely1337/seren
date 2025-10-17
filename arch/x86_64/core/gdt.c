#define pr_fmt(fmt) "gdt: " fmt

#include <asm/gdt.h>
#include <lib/string.h>
#include <nucleus/printk.h>

extern void gdt_flush(struct gdt_ptr *gdtp);
extern void tss_flush(u16 selector);

static struct tss tss __attribute__((aligned(16)));
static u8 ist_double_fault_stack[IST_STACK_SIZE] __attribute__((aligned(16)));

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtp;

/*
 * gdt_set_entry - Configure a flat 64-bit GDT descriptor.
 */
static void gdt_set_entry(int num, u8 access, u8 flags) {
    gdt[num].limit0 = 0xFFFF;
    gdt[num].base0 = 0;
    gdt[num].base1 = 0;
    gdt[num].access_byte = access;
    gdt[num].limit1_flags = 0x0F | flags;
    gdt[num].base2 = 0;
}

/*
 * tss_set_entry - Configure the 16-byte TSS descriptor
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

void gdt_init(void) {
    memset(&tss, 0, sizeof(tss));
    tss.ist[IST_DOUBLE_FAULT - 1] =
        (u64)ist_double_fault_stack + IST_STACK_SIZE;

    gdt_set_entry(GDT_KERNEL_CODE_SELECTOR / 8,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
                      GDT_ACCESS_TYPE_CODE_DATA | GDT_ACCESS_TYPE_CODE_READ |
                      GDT_ACCESS_TYPE_CODE_EXEC,
                  GDT_FLAG_GRAN_4K | GDT_FLAG_64BIT);

    gdt_set_entry(GDT_KERNEL_DATA_SELECTOR / 8,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
                      GDT_ACCESS_TYPE_CODE_DATA | GDT_ACCESS_TYPE_DATA_WRITE,
                  GDT_FLAG_GRAN_4K | GDT_FLAG_32BIT); /* Data segs use D/B=1 */

    gdt_set_entry(GDT_USER_CODE_SELECTOR / 8,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
                      GDT_ACCESS_TYPE_CODE_DATA | GDT_ACCESS_TYPE_CODE_READ |
                      GDT_ACCESS_TYPE_CODE_EXEC,
                  GDT_FLAG_GRAN_4K | GDT_FLAG_64BIT);

    gdt_set_entry(GDT_USER_DATA_SELECTOR / 8,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
                      GDT_ACCESS_TYPE_CODE_DATA | GDT_ACCESS_TYPE_DATA_WRITE,
                  GDT_FLAG_GRAN_4K | GDT_FLAG_32BIT);

    tss_set_entry(GDT_TSS_SELECTOR / 8, (u64)&tss);

    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base = (u64)&gdt;

    gdt_flush(&gdtp);
    tss_flush(GDT_TSS_SELECTOR);

    pr_info("GDT and TSS initialized and loaded\n");
}