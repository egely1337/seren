// SPDX-License-Identifier: Apache-2.0

#ifndef _ASM_GENERIC_INT_LL64_H
#define _ASM_GENERIC_INT_LL64_H

#include <uapi/asm-generic/int-ll64.h>

/**
 * These are the short, kernel-style typedefs for fixed-width integers.
 * They are aliases for the __u8, __s32 types defined in the uapi header.
 *
 * This separation allows kernel code to use the convenient short names while
 * keeping the namespace-prefixed names available for user-space compatibility.
 */

typedef __s8 s8;
typedef __u8 u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;

#endif // _ASM_GENERIC_INT_LL64_H