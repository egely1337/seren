#ifndef ARCH_X86_64_ARCH_H
#define ARCH_X86_64_ARCH_H

#define sti() __asm__ volatile ("sti");
#define cli() __asm__ volatile ("cli");
#define hlt() __asm__ volatile ("hlt");

void arch_init(void);

#endif // ARCH_X86_64_ARCH_H