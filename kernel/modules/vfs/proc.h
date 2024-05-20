#ifndef VFS_PROC_H
#define VFS_PROC_H

#include "../vfs.h"

extern iodriver *procfs_driver;
extern filesystem *procfs;

void proc_init(void);

#endif // VFS_PROC_H
