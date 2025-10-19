// SPDX-License-Identifier: Apache-2.0

#ifndef _NUCLEUS_COMPILER_H
#define _NUCLEUS_COMPILER_H

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // _NUCLEUS_COMPILER_H