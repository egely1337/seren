// SPDX-License-Identifier: Apache-2.0

#ifndef _DEVICE_FS_H
#define _DEVICE_FS_H

struct device;
typedef void (*device_close_op)(const char *);
typedef void (*device_read_op)(const char *);
typedef void (*device_write_op)(const char *, struct device *, const void *);
typedef void (*device_touch_op)(const char *, struct device *, const void *);
typedef void (*device_mount_op)(struct device *);
typedef void (*device_probe_op)(struct device *);

struct device {
	char *name;
	struct {
		device_read_op read;
		device_write_op write;
	} ops;
};
#endif