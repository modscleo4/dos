#ifndef VFS_TMP_H
#define VFS_TMP_H

#include "../vfs.h"

extern iodriver *tmpfs_driver;
extern filesystem *tmpfs;

void tmp_init(void);

#endif // VFS_TMP_H
