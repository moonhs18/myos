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

struct vnode;

//VFS polymorphic interface via function pointer table
typedef struct vnode_ops{
    int (*open)(struct vnode *node, int flags);
    int64_t (*read)(struct vnode *node, uint64_t offset, void *buf, size_t count);
    int64_t (*write)(struct vnode *node, uint64_t offset, void *buf, size_t count);
    int (*close)(struct vnode *node);
    struct vnode* (*lookup)(struct vnode *parent, const char *name);
    int (*create)(struct vnode *parent, const char *name, vnode_type_t type, struct vnode **out_node);
} vnode_ops_t;

//Virtual File System Node(vnode)
typedef struct vnode{
    char name[MAX_FILENAME];
    vnode_type_t type;
    size_t size;
    void *interanl_data;
    vnode_ops_t *ops;
    struct vnode *parent;
    struct vnode *child_head;
    struct vnode *next_sibling;
} vnode_t;

//Open File Handle structure
typedef struct file{
    vnode_t *vnode;
    uint64_t pos;   //current file r/w offset
    int flags;      //open mode flag
    int ref_count;  //reference count
} file_t;

//VFS open API
void vfs_init(void);
vnode_t* vfs_get_root(void);
vnode_t* vfs_lookup(const char *path);
file_t* vfs_open(const char *path, int flags);
int64_t vfs_read(file_t *file, void *buf, size_t count);
int64_t vfs_write(file_t *file, const void *buf, size_t count);
int vfs_close(file_t *file);

#endif