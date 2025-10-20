// SPDX-License-Identifier: Apache-2.0

/**
 * This header defines our "initcall" mechanism, which is heavily inspired
 * by the Linux kernel. It allows different parts of the kernel to register
 * an initialization function to be called automatically during boot without
 * needing a giant list of `init_everything()` calls.
 */

#define __section(section) __attribute__((__section__(section)))

typedef int (*initcall_t)(void);

/**
 * This is the core of the initcall magic. It defines a function pointer
 * of type `initcall_t` and places it in a specially named ELF section.
 * For example, a call like `core_initcall(foo)` will create a pointer to `foo`
 * and place it in the `initcall1.init` section.
 */
#define __define_initcall(fn, level)                                           \
	static initcall_t __initcall_##fn##_##level __attribute__((            \
	    __used__, __section__(".initcall" #level ".init"))) = fn

#ifndef __init
#define __init __section(".init.text")
#endif

extern initcall_t __initcall_start;
extern initcall_t __initcall_end;

/* Level 0: Pure initialization, very early. No dependencies. */
#define pure_initcall(fn) __define_initcall(fn, 0)
/* Level 1: Core subsystems, like the PMM. */
#define core_initcall(fn) __define_initcall(fn, 1)
/* Level 2: Things that depend on core subsystems, like the slab allocator. */
#define postcore_initcall(fn) __define_initcall(fn, 2)
/* Level 3: Architecture-specific setup. */
#define arch_initcall(fn) __define_initcall(fn, 3)
/* Level 4: Major subsystems, like the VFS. */
#define subsys_initcall(fn) __define_initcall(fn, 4)
/* Level 5: Filesystem drivers. */
#define fs_initcall(fn) __define_initcall(fn, 5)
/* Level 6: Most device drivers, which run late in the process. */
#define device_initcall(fn) __define_initcall(fn, 6)