// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 egely1337
 */

#define pr_fmt(fmt) "vfs: " fmt

#include <lib/string.h>
#include <seren/fs/vfs.h>
#include <seren/init.h>
#include <seren/mm.h>
#include <seren/printk.h>

#define MAX_MOUNTS (int)32

struct filesystem *vfs_root = NULL;
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

u8 __check_is_mounted(const char* path) {
	struct mount_info* mp;
	
	for(volatile size_t i = 0; i < mounted; ++i) {
		mp = mount_points[i];
		
		if(mp) {
			if(strcmp(path, mp->mount_point) == 0)
				return 0;
		}
	}

	return 1;
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
	
	/* Check provided filesystem */
	if (NULL == rootfs || 
		NULL != mount_points ||
		mounted > 0
	) {
		goto fail;
	}

	/* Make our Root Filesystem to provided filesystem. */
	vfs_root = rootfs;

	/* Initialize RootFS at mount_points */
	struct mount_info *root = mount_points[mounted];
	root->mount_point = "/";
	root->fs = rootfs;
	mounted++;

fail:
	pr_emerg("Mount root failed, check vfs_mount_root() parameters.\n");
	return;
}

int vfs_mount(const char* path, struct filesystem* fs) {
	if(unlikely(__check_is_mounted(path) == VFS_FAIL)) {
		pr_emerg("Some fs already mounted to this path: %s\n", path);
		return VFS_FAIL;
	}
	
	/* Allocate new mount point */
	struct mount_info* new_mount_info = mount_points[mounted];
	new_mount_info = (struct mount_info*)kmalloc(sizeof(struct mount_info));

	/* Allocation failed, Check errors */
	if(!new_mount_info) {
		pr_emerg("Out of memory to allocate new mount point.\n");
		return VFS_FAIL;
	}

	/* Setup new mount point */
	new_mount_info->fs = fs;
	new_mount_info->mount_point = (char*)path;
	return VFS_SUCCESS;
}

static int __init setup_vfs(void) {
	vfs_init();
	return 0;
}

subsys_initcall(setup_vfs);