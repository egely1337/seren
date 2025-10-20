// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_COMPILER_H
#define _SEREN_COMPILER_H

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // _SEREN_COMPILER_H