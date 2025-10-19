// SPDX-License-Identifier: Apache-2.0

#ifndef _NUCLEUS_TYPES_H
#define _NUCLEUS_TYPES_H

#include <nucleus/compiler.h>
#include <uapi/nucleus/types.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

#if defined(__x86_64__) || defined(__aarch64__)
typedef u64 size_t;
typedef u64 uintptr_t;
typedef s64 ssize_t;
typedef s64 intptr_t;
#else
#error "Unsupported architecture"
#endif

#ifndef __cplusplus
typedef _Bool bool;
enum { false = 0, true = 1 };
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef __s_size_t size_t;
#endif

#ifndef _PID_T
#define _PID_T
typedef __s_pid_t pid_t;
#endif

typedef u64 phys_addr_t;

#endif // _NUCLEUS_TYPES_H