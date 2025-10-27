// SPDX-License-Identifier: Apache-2.0

#ifndef _RAMFS_
#define _RAMFS_

#include <seren/fs/devicefs.h>
#include <lib/string.h>
#include <seren/types.h>

typedef enum { SEEK_SET, SEEK_START, SEEK_END } ramfs_seek_t;

/**
 * @brief Our device descriptor
 */
struct ramfs {
	void *buf;
	size_t len;
	size_t current;
};

/**
 * @brief Initializes RamFS
 */
struct device *ramfs_init(void);

/**
 * @brief Read from RamFS
 */
int ramfs_read(struct device *dev, const void *buf, size_t len);

/**
 * @brief Seek RamFS file
 */
int ramfs_write(struct device *dev, size_t size);

#endif