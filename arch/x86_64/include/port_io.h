#include <nucleus/printk.h>

#ifndef X86_64_PORT_IO_H
#define X86_64_PORT_IO_H

#include <nucleus/types.h>

static inline void outb(u16 port, u8 value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) { outb(0x80, 0); }

#endif // ARCH_X86_64_PORT_IO_H