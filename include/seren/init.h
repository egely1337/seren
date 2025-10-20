// SPDX-License-Identifier: Apache-2.0

#define __section(section) __attribute__((__section__(section)))

typedef int (*initcall_t)(void);

#define __define_initcall(fn, level)                                           \
	static initcall_t __initcall_##fn##_##level __attribute__((            \
	    __used__, __section__(".initcall" #level ".init"))) = fn

#ifndef __init
#define __init __section(".init.text")
#endif

extern initcall_t __initcall_start;
extern initcall_t __initcall_end;

#define pure_initcall(fn)     __define_initcall(fn, 0)
#define core_initcall(fn)     __define_initcall(fn, 1)
#define postcore_initcall(fn) __define_initcall(fn, 2)
#define arch_initcall(fn)     __define_initcall(fn, 3)
#define subsys_initcall(fn)   __define_initcall(fn, 4)
#define fs_initcall(fn)	      __define_initcall(fn, 5)
#define device_initcall(fn)   __define_initcall(fn, 6)