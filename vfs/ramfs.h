#ifndef RAMFS_H
#define RAMFS_H

#include "vfs.h"

void ramfs_init(void);
vnode_t* ramfs_mount(const char *mount_point);

#endif