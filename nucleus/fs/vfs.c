#define pr_fmt(fmt) "vfs: " fmt

#include <nucleus/fs/vfs.h>
#include <nucleus/mm/slab.h>
#include <nucleus/printk.h>


struct vfs* root = NULL;
struct mount_info* mount_head = NULL;

void vfs_init(void) {
    /* 
        Just a playground, not a serious code.
    */

    root = (struct vfs*)kmalloc(sizeof(struct vfs));
    root->name = "(root)";
    mount_head = (struct mount_info*)kmalloc(sizeof(struct mount_info));
    INIT_LIST_HEAD(&mount_head->head);
    pr_debug("allocated root at %p\n", root);
}