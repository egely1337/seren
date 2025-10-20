// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_COMPILER_H
#define _SEREN_COMPILER_H

/**
 * These macros provide optimization hints to the compiler about the expected
 * outcome of a condition. They don't change the logic of the code, but they can
 * help the compiler generate slightly more efficient machine code by arranging
 * branches to favor the common case.
 *
 * Use likely(expr) if you expect `expr` to be true most of the time.
 * Use unlikely(expr) if you expect `expr` to be false most of the time.
 *
 * Example:
 * 	if (unlikely(error_condition)) {
 * 		handle_error();
 * 	}
 */

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // _SEREN_COMPILER_H