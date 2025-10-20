// SPDX-License-Identifier: Apache-2.0

#ifndef __ASM_GENERIC_POSIX_TYPES_H
#define __ASM_GENERIC_POSIX_TYPES_H

/**
 * This file provides default definitions for core POSIX types.
 * Architectures can override these if they need a different size,
 * but for most modern systems these are the correct definitions.
 */

#ifndef __s_pid_t
typedef int __s_pid_t;
#endif

#ifndef __s_size_t
typedef unsigned long long __s_size_t;
#endif

#endif // __ASM_GENERIC_POSIX_TYPES_H