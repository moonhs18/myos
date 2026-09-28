#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PATH_LEN    128
#define MAX_FILENAME    32
#define MAX_OPEN_FILES  32

//File Access FLAG
#define O_RDONLY        0x000
#define O_WRONLY        0x001
#define O_RDWR          0x002
#define O_CREAT         0x040
#define O_TRUNC         0x200

//vnode type 
typedef enum {
    VNODE_TYPE_FILE,
    VNODE_TYPE_DIR,
    VNODE_TYPE_DEV
} vnode_type_t;

struct vnode










#endif