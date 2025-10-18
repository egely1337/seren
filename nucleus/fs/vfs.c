#define pr_fmt(fmt) "vfs: " fmt

#include <nucleus/fs/vfs.h>
#include <nucleus/mm/slab.h>
#include <nucleus/printk.h>


struct filesystem* root = NULL;
struct mount_info* mount_head = NULL;

void vfs_init(void) {
    /* 
        Just a playground, not a serious code.
    */
}