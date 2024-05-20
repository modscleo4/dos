#ifndef VFS_DEV_H
#define VFS_DEV_H

#include "../vfs.h"

extern iodriver *devfs_driver;
extern filesystem *devfs;

void dev_init(void);

#endif // VFS_DEV_H
