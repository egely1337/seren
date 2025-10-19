// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 egely1337
 */

#define pr_fmt(fmt) "vfs: " fmt

#include <lib/string.h>
#include <nucleus/fs/vfs.h>
#include <nucleus/mm/slab.h>
#include <nucleus/printk.h>

#define MAX_MOUNTS (int)32

struct filesystem *root = NULL;
struct mount_info **mount_points = NULL;
size_t mounted = 0;

void vfs_init(void) {
	if (mounted != 0) {
		pr_crit("Already inited.\n");
	}

	mount_points = (struct mount_info **)kmalloc(sizeof(struct mount_info) *
						     MAX_MOUNTS);
	if (!mount_points) {
		pr_emerg("Mount points could not be allocated.\n");
	}

	pr_info("Virtual File System initialized.\n");
	pr_info("Now mount a rootfs.\n");
}

bool __str_backspace(char *str, char c) {
	size_t i = strlen(str);
	i--;
	while (i) {
		i--;
		if (str[i] == c) {
			str[i + 1] = 0;
			return true;
		}
	}

	return false;
}

u8 __find_mount(char *filename, int *adjust) {
	char *orig = (char *)kmalloc(strlen(filename) + 1);
	memset(orig, 0, strlen(filename) + 1);
	memcpy(orig, filename, strlen(filename) + 1);
	if (orig[strlen(orig)] == '/')
		__str_backspace(orig, '/');

	while (1) {
		for (int i = 0; i < MAX_MOUNTS; i++) {
			if (!mount_points[i])
				break;
			if (strcmp(mount_points[i]->mount_point, orig) == 0) {
				*adjust = (strlen(orig) - 1);
				kfree(orig);
				return i;
			}
		}
		if (strcmp(orig, "/") == 0)
			break;
		__str_backspace(orig, '/');
	}

	return 0;
}

void vfs_mount_root(struct filesystem *rootfs) {
	if (rootfs == NULL) {
		pr_emerg("Provided rootfs is NULL.\n");
	}

	if (mount_points != NULL) {
		pr_crit("VFS is not initialized.\n");
	}

	root = rootfs;
	if (mounted != 0) {
		goto already_mounted;
	}

	/* Initialize RootFS */
	struct mount_info *root = mount_points[0];
	root->mount_point = "/";
	root->fs = rootfs;

already_mounted:
	pr_crit("Root may be already mounted.\n");
}