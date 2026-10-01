#include "ramfs.h"
#include "heap.h"
#include "uart.h"

typedef struct ramfs_node{
    uint8_t *data;
    size_t capacity;
}ramfs_node_t;

static void ramfs_memcpy(void *dest, const void *src, size_t n){
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while(n--) *d++ = *s++;
}

static int ramfs_open(vnode_t *node, int flags){
    (void)flags;
    return (node !=NULL) ? 0 : -1;
}

static int64_t ramfs_read(vnode_t *node, uint64_t offset, void *buf, size_t count){
    if(!node || node->type != VNODE_TYPE_FILE) return -1;
    if(offset >= node->size) return 0; // EOF

    size_t read_bytes = count;
    if(offset + read_bytes > node->size){
        read_bytes = node->size - offset;
    }

    ramfs_node_t *rdata = (ramfs_node_t *)node->interanl_data;
    if(!rdata || !rdata->data) return -1;

    ramfs_memcpy(buf, rdata->data + offset, read_bytes);
    return read_bytes;
}

static int64_t ramfs_write(vnode_t *node, uint64_t offset, const void *buf, size_t count){
    if(!node || node->type !=VNODE_TYPE_FILE) return -1;
    
    ramfs_node_t *rdata = (ramfs_node_t *)node->interanl_data;
    if(!rdata) return -1;

    if(offset + count > rdata->capacity){
        size_t new_cap = (offset + count) * 2;
        if(new_cap < 128) new_cap = 128;

        uint8_t *new_buf = (uint8_t *)kmalloc(new_cap);
        if(!new_buf) return -1;

        if(rdata->data){
            ramfs_memcpy(new_buf, rdata->data, node->size);
            kfree(rdata->data);
        }
        rdata->data = new_buf;
        rdata->capacity = new_cap;
    }

    ramfs_memcpy(rdata->data + offset, buf, count);
    if(offset + count > node->size){
        node->size = offset + count;
    }
    return count;
}

static int ramfs_close(vnode_t *node){
    (void)node;
    return 0;
}

static int ramfs_create(vnode_t *parent, const char *name, vnode_type_t type, vnode_t **out_node);

static vnode_ops_t ramfs_ops = {
    .open = ramfs_open,
    .read = ramfs_read,
    .write = ramfs_write,
    .close = ramfs_close,
    .lookup = NULL,
    .create = ramfs_create
};

static int ramfs_create(vnode_t *parent, const char *name, vnode_type_t type, vnode_t **out_node){
    extern vnode_t* vfs_create_vnode(const char *name, vnode_type_t type, vnode_ops_t *ops);
    extern void vfs_add_child(vnode_t *parent, vnode_t *child);

    vnode_t *child = vfs_create_vnode(name, type, &ramfs_ops);
    if(!child) return -1;

    if(type == VNODE_TYPE_FILE){
        ramfs_node_t *rdata = (ramfs_node_t *)kmalloc(sizeof(ramfs_node_t));
        rdata->data = NULL;
        rdata->capacity = 0;
        child->interanl_data = rdata;
    }

    vfs_add_child(parent, child);
    if(out_node) *out_node = child;

    return 0;
}

vnode_t* ramfs_mount(const char *mount_point){
    vnode_t *root = vfs_get_root();
    vnode_t *mnt_node = NULL;

    if(ramfs_create(root, mount_point, VNODE_TYPE_DIR, &mnt_node) < 0){
        uart_puts("[RamFS ERROR] Failed to mount RamFS.\n");
        return NULL;
    }

    uart_puts("[RamFS] Mounted successfully at /");
    uart_puts(mount_point);
    uart_puts("\n");

    return mnt_node;
}

void ramfs_init(void){
    ramfs_mount("ram");
}
