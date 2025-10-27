// SPDX-License-Identifier: Apache-2.0

#ifndef _DEVICE_FS_H
#define _DEVICE_FS_H

#include <seren/list.h>

struct device;
typedef void (*device_read_op)	(struct device*, const char*);
typedef void (*device_write_op)	(struct device*, const char*);

struct device {
	char 	*name;
	void 	*devptr;
	struct {
		device_read_op read;
		device_write_op write;
	} ops;
	struct list_head dev_list;
};

/**
 * @brief Initializes devicefs
 */
int devicefs_init(void);

/**
 * @brief Adds device to devicefs
 * @param name Name of the device
 * @param device Address of the device structure
 * @param read Address of the read operation
 * @param write Address of the write operation
 */
struct device* devicefs_add(const char* name, void* device, device_read_op read, device_write_op write);

/**
 * @brief Device read
 * @param path Device path
 * @param buf Buffer
 * @param count Count
 */
void devicefs_read(const char* path, const void* buf, size_t count); 

#endif