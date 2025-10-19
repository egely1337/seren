// SPDX-License-Identifier: Apache-2.0

#ifndef _RAMFS_
#define _RAMFS_

#include <lib/string.h>
#include <nucleus/fs/devicefs.h>
#include <nucleus/types.h>

typedef enum { SEEK_SET, SEEK_START, SEEK_END } ramfs_seek_t;
struct ramfs {
	void *buf;
	size_t len;
	size_t current;
};

/**
 * @brief Initializes RamFS
 */
struct ramfs *ramfs_init(void);

/**
 * @brief Read from RamFS
 */
void ramfs_read(struct ramfs *ramfs, const void *buf, size_t len);

/**
 * @brief Seek RamFS file
 */
void ramfs_seek(struct ramfs *ramfs, ramfs_seek_t type, size_t size);

#endif