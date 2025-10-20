// SPDX-License-Identifier: Apache-2.0

#ifndef STRING_H
#define STRING_H

#include <seren/stddef.h>
#include <seren/types.h>

/**
 * strlen - Calculate the length of a string.
 * @s: The null-terminated string to measure.
 * Returns the number of characters in s, not including the terminator.
 */
size_t strlen(const char *s);

/**
 * strcpy - Copy a string.
 * @dest: The destination buffer.
 * @src: The source string.
 * Returns a pointer to dest.
 *
 * NOTE: Does not check for buffer overflows! Use strncpy for safer copies.
 * The memory areas of src and dest must not overlap.
 */
char *strcpy(char *dest, const char *src);

/**
 * strncpy - Copy a string with a size limit.
 * @dest: The destination buffer.
 * @src: The source string.
 * @n: The maximum number of characters to copy.
 * Returns a pointer to dest.
 *
 * NOTE: If src is longer than n, dest will not be null-terminated.
 * If src is shorter than n, the remainder of dest is padded with nulls.
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * strcat - Concatenate (append) two strings.
 * @dest: The destination string. Must have enough space for the result.
 * @src: The string to append.
 * Returns a pointer to dest.
 */
char *strcat(char *dest, const char *src);

/**
 * strncat - Concatenate two strings with a size limit.
 * @dest: The destination string.
 * @src: The source string.
 * @n: The maximum number of characters to append from src.
 * Returns a pointer to dest. Always null-terminates.
 */
char *strncat(char *dest, const char *src, size_t n);

/**
 * strcmp - Compare two strings lexicographically.
 * @s1: The first string.
 * @s2: The second string.
 * Returns 0 if strings are equal, <0 if s1 < s2, >0 if s1 > s2.
 */
int strcmp(const char *s1, const char *s2);

/**
 * strncmp - Compare two strings up to n characters.
 * @s1: The first string.
 * @s2: The second string.
 * @n: The maximum number of characters to compare.
 * Returns 0 if strings are equal, <0 if s1 < s2, >0 if s1 > s2.
 */
int strncmp(const char *s1, const char *s2, size_t n);

/**
 * memcpy - Copy a region of memory.
 * @dest: The destination memory area.
 * @src: The source memory area.
 * @n: The number of bytes to copy.
 * Returns a pointer to dest.
 *
 * NOTE: The memory areas must not overlap. Use memmove if they might.
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * memset - Fill a region of memory with a constant byte.
 * @s: The memory area to fill.
 * @c: The byte to fill with.
 * @n: The number of bytes to fill.
 * Returns a pointer to s.
 */
void *memset(void *s, int c, size_t n);

/**
 * memcmp - Compare two regions of memory.
 * @s1: The first memory area.
 * @s2: The second memory area.
 * @n: The number of bytes to compare.
 * Returns 0 if they are equal, <0 if s1 < s2, >0 if s1 > s2.
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * memmove - Copy a region of memory, handling overlapping areas correctly.
 * @dest: The destination memory area.
 * @src: The source memory area.
 * @n: The number of bytes to copy.
 * Returns a pointer to dest.
 */
void *memmove(void *dest, const void *src, size_t n);

/**
 * strchr - Find the first occurrence of a character in a string.
 * @s: The string to search.
 * @c: The character to find.
 * Returns a pointer to the first occurrence of c, or NULL if not found.
 */
char *strchr(const char *s, int c);

#endif // STRING_H