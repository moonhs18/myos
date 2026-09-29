#include "devfs.h"
#include "uart.h"

static int devfs_console_open(vnode_t *node, int flags){
    (void)node;
    (void)flags;
    return 0;    
}

static int64_t devfs_console_read(vnode_t *node, uint64_t offset, void *buf, size_t count){
    (void)node;
    (void)offset;
    (void)buf;
    (void)count;

    return 0;
}
static int64_t devfs_console_write(vnode_t *node, uint64_t offset, const void *buf, size_t count){
    (void)node;
    (void)offset;

    const char *str = (const char *)buf;
    for(size_t i = 0; i < count; i++){
        uart_putc(str[i]);
    }

    return count;
}

static int devfs_console_close(vnode_t *node){
    (void)node;
    return 0;
}

static vnode_ops_t devfs_console_ops = {
    .open = devfs_console_open,
    .read = devfs_console_read,
    .write = devfs_console_write,
    .close = devfs_console_close,
    .lookup = NULL,
    .create = NULL
};

void devfs_init(void){
    extern vnode_t* vfs_create_vnode(const char *name, vnode_type_t type, vnode_ops_t *ops);
    extern void vfs_add_child(vnode_t *parent, vnode_t *child);

    vnode_t *root = vfs_get_root();

    //create /dev directory
    vnode_t *dev_dir = vfs_create_vnode("dev", VNODE_TYPE_DIR, NULL);
    vfs_add_child(root, dev_dir);

    // create /dev/console device node and UART driver mapping
    vnode_t *console_node = vfs_create_vnode("console",VNODE_TYPE_DEV, &devfs_console_ops);
    vfs_add_child(dev_dir, console_node);

    uart_puts("[DevFS] Mounted /dev/console (PL011 UART Device Node).\n");
}
