// SPDX-License-Identifier: Apache-2.0

#ifndef _VFS_H
#define _VFS_H

#include <seren/fs/devicefs.h>
#include <seren/list.h>
#include <seren/types.h>

#define VFS_SUCCESS 0
#define VFS_FAIL 1

struct device;
typedef void (*read_op)		(struct device*, const char *, const void* buf, size_t count);
typedef void (*write_op)	(struct device*, const char *, const void *);
typedef void (*touch_op)	(struct device*, const char*);
typedef void (*mount_op)	(struct device*);
typedef void (*probe_op)	(struct device*);
typedef void (*close_op)	(struct device*);

struct filesystem {
	char *name;
	struct {
		read_op read;
		close_op close;
		touch_op touch;
		probe_op probe;
		mount_op mount;
		write_op write;
	} ops;
};

struct mount_info {
	char *mount_point;
	struct filesystem *fs;
};

/**
 * @brief Initializes virtual filesystem
 */
void vfs_init(void);

/**
 * @brief Read from virtual filesystem
 */
void vfs_read(const char *path, void *buf);

/**
 * @brief Write to virtual filesystem
 */
void vfs_write(const char *path, void *buf, size_t n);

/**
 * @brief Mounts root
 */
void vfs_mount_root(struct filesystem *rootfs);

/**
 * @brief Mounts path
 */
int vfs_mount(const char* path, struct filesystem* fs);

#endif