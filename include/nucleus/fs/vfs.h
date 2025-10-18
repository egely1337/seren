#ifndef _VFS_H
#define _VFS_H

#include <nucleus/types.h>
#include <nucleus/fs/devicefs.h>
#include <nucleus/list.h>

struct device;
typedef void (*close_op)(const char*);
typedef void (*read_op)(const char*);
typedef void (*write_op)(const char*, struct device*, const void*);
typedef void (*touch_op)(const char*, struct device*, const void*);
typedef void (*mount_op)(struct device*);
typedef void (*probe_op)(struct device*);

struct filesystem {
    char* name;
    struct
    {    
        read_op read;
        close_op close;
        touch_op touch;
        probe_op probe;
        mount_op mount;
        write_op write;
    } ops;
};

struct mount_info {
    char* mount_point;
    struct filesystem* fs;
};

/**
 * @brief Initializes virtual filesystem
 */
void vfs_init(void);

/**
 * @brief Read from virtual filesystem
 */
void vfs_read(const char* path, void* buf);

/**
 * @brief Write to virtual filesystem
 */
void vfs_write(const char* path, void* buf, size_t n);

/**
 * @brief Mounts root
 */
void vfs_mount_root(struct filesystem* rootfs);

#endif