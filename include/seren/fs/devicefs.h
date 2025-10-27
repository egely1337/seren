// SPDX-License-Identifier: Apache-2.0

#ifndef _DEVICE_FS_H
#define _DEVICE_FS_H

#include <seren/list.h>

struct device;
typedef void (*device_close_op)(const char *);
typedef void (*device_read_op)(const char *);
typedef void (*device_write_op)(const char *, struct device *, const void *);
typedef void (*device_touch_op)(const char *, struct device *, const void *);
typedef void (*device_mount_op)(struct device *);
typedef void (*device_probe_op)(struct device *);

struct device {
	char *name;
	void* devptr;
	struct {
		device_read_op read;
		device_write_op write;
	} ops;
	struct list_head dev_list;
};

/**
 * @brief Initializes devicefs
 */
void devicefs_init(void);

/**
 * @brief Adds device to devicefs
 * @param name Name of the device
 * @param device Address of the device structure
 * @param read Address of the read operation
 * @param write Address of the write operation
 */
void devicefs_add(const char* name, const void* device, device_read_op read, device_write_op write);

#endif