#include "vfs.h"
#include "heap.h"
#include "uart.h"

static vnode_t *root_vnode = NULL;

//String utility 
static int vfs_strcmp(const char *s1, const char *s2){
    while(*s1 && (*s1 == *s2)) {s1++, s2++;}
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


static void vfs_strncpy(char *dest, const char *src, size_t n){
    size_t i;
    for(i = 0; i < n-1 && src[i] != '\0'; i++){
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

vnode_t* vfs_create_vnode(const char *name, vnode_type_t type, vnode_ops_t *ops){
    vnode_t *node = (vnode_t *)kmalloc(sizeof(vnode_t));
    if(!node) return NULL;

    vfs_strncpy(node->name, name, MAX_FILENAME);
    node->type = type;
    node->size = 0;
    node->interanl_data = NULL;
    node->ops = ops;
    node->parent = NULL;
    node->child_head = NULL;
    node->next_sibling = NULL;

    return node;
}

void vfs_add_child(vnode_t *parent, vnode_t *child){
    if(!parent || !child) return;
    child->parent = parent;
    child->next_sibling = parent->child_head;
    parent->child_head = child;
}


//path parsing and hierarchical vnode lookup engine
void* vfs_lookup(const char *path){
    if(!path || path[0] != '/') return NULL;
}