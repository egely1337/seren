#define pr_fmt(fmt) "devicefs: " fmt

#include <seren/fs/devicefs.h>
#include <seren/printk.h>
#include <seren/init.h>
#include <seren/mm.h>
#include <seren/list.h>


/* Root device */
struct device * devicefs_root = NULL;

/**
 * @brief Initializes devicefs
 */
int devicefs_init(void) {
    if(NULL != devicefs_root) {
        pr_emerg("devicefs already initialized\n");
        return 1;
    }

    /* Allocate devicefs */
    devicefs_root = (struct device*)kmalloc(sizeof(struct device));
    
    /* Check allocation successful */
    if(NULL == devicefs_root) {
        pr_emerg("Not enough memory!");
        return 1;
    }

    /* Setup devicefs_root device */
    devicefs_root->name = "(root)";
    devicefs_root->devptr = NULL;
    devicefs_root->ops.read = NULL;
    devicefs_root->ops.write = NULL;
    INIT_LIST_HEAD(&devicefs_root->dev_list);
    
    return 0;
}

/**
 * @brief Adds device to devicefs
 * @param name Name of the device
 * @param device Address of the device structure
 * @param read Address of the read operation
 * @param write Address of the write operation
 */
void devicefs_add(const char* name, const void* device, device_read_op read, device_write_op write);


static int __init devicefs_setup() {
    if(likely(devicefs_init() == 0)) {
        pr_info("devicefs has initialized\n");
    }

    return 0;
}

subsys_initcall(devicefs_setup);