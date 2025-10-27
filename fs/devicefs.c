#define pr_fmt(fmt) "devicefs: " fmt

#include <seren/printk.h>
#include <seren/init.h>
#include <seren/mm/slab.h>
#include <seren/list.h>





static int __init devicefs_setup() {
    pr_info("hello world!\n");
    return 0;
}

subsys_initcall(devicefs_setup);