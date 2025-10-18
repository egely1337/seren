#include <nucleus/fs/ext2/ext2.h>
#include <nucleus/fs/devicefs.h>


struct ext2_sb* superblock = (void*)0;
bool __initialized = false;

struct filesystem *ext2_init(struct device* dev) {
   (void)dev;
    return 0;
}