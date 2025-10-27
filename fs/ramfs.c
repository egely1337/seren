// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 egely1337
 */

#include <seren/mm.h>
#include <seren/fs/devicefs.h>
#include <seren/fs/ramfs.h>


struct device *ramfs_init(void) {
    /* Create ramfs descriptor */
    struct ramfs* fs = (struct ramfs*)kmalloc(sizeof(struct ramfs));

    if(!fs) {
        // TODO: handle here
    }
 
    /* Add device here with proper functions */
    return devicefs_add("ramfs", (void*)fs, NULL, NULL);
}