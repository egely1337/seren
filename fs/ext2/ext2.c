// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 egely1337
 */

#include <seren/fs/devicefs.h>
#include <seren/fs/ext2/ext2.h>

struct ext2_sb *superblock = (void *)0;
bool __initialized = false;

struct filesystem *ext2_init(struct device *dev) {
	(void)dev;
	return 0;
}
