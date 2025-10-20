// SPDX-License-Identifier: Apache-2.0

#ifndef FORMAT_H
#define FORMAT_H

#include <lib/stdarg.h>
#include <seren/stddef.h>
#include <seren/types.h>

/**
 * itoa_k - Convert a long long to a string.
 * @value: The number to convert.
 * @str: The buffer to store the resulting string in.
 * @base: The numerical base (e.g., 10 for decimal, 16 for hex).
 * @is_signed: Treat the value as signed (adds a '-' if negative).
 * @min_width: The minimum width of the output. Padded if necessary.
 * @pad_char: The character to use for padding (e.g., ' ' or '0').
 */
char *itoa_k(long long value, char *str, int base, int is_signed, int min_width,
	     char pad_char);

/**
 * kvsnprintf - Format a string from a va_list into a sized buffer.
 * @buf: The destination buffer.
 * @size: The total size of the destination buffer, including the null
 * terminator.
 * @fmt: The printf-style format string.
 * @args: The va_list of arguments.
 */
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/**
 * ksnprintf - Format a string into a sized buffer.
 * @buf: The destination buffer.
 * @size: The total size of the buffer.
 * @fmt: The format string.
 * @...: The arguments to format.
 */
int ksnprintf(char *buf, size_t size, const char *fmt, ...);

#endif