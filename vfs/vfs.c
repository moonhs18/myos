#include "vfs.h"
#include "heap.h"
#include "uart.h"

static vnode_t *root_vnode = NULL;

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

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
vnode_t* vfs_lookup(const char *path){
    if(!path || path[0] != '/') return NULL;
    if(path[1] == '\0') return root_vnode;

    vnode_t *curr = root_vnode;
    char token[MAX_FILENAME];
    const char *p = path + 1;

    while (*p)
    {
        size_t len = 0;
        while (*p && *p != '/'){
            if(len <MAX_FILENAME - 1){
                token[len++] = *p;
            }
            p++;
        }
        token[len] = '\0';
        if(*p == '/') p++;

        if(len == 0) continue;

        //Lookup a child node from the curr dir node
        if(curr->type != VNODE_TYPE_DIR) return NULL;
        
        vnode_t *child = curr->child_head;
        vnode_t *found = NULL;
        
        while (child)
        {
            if(vfs_strcmp(child->name, token) == 0){
                found = child;
                break;
            }
            child = child->next_sibling;
        }
        
        if(!found){
            if(curr->ops && curr->ops->lookup){
                found = curr->ops->lookup(curr, token);
            }
        }

        if(!found) return NULL;
        curr = found;
    }
    
    return curr;
}

file_t* vfs_open(const char *path, int flags){
    vnode_t *node = vfs_lookup(path);
    
    if(!node && (flags & O_CREAT)){
        char parent_path[MAX_PATH_LEN];
        char filename[MAX_FILENAME];

        vfs_strncpy(parent_path, path, MAX_PATH_LEN);
        int last_slash = -1;
        for(int i=0; parent_path[i] != '\0';i++){
            if(parent_path[i] == '/') last_slash = i;
        }

        if(last_slash == 0){
            vfs_strncpy(parent_path, "/", MAX_PATH_LEN);
            vfs_strncpy(filename, path + 1, MAX_FILENAME);
        }
        else if (last_slash > 0)
        {
            parent_path[last_slash] = '\0';
            vfs_strncpy(filename, path + last_slash + 1, MAX_FILENAME);
        }
        else{
            return NULL;
        }

        vnode_t *parent = vfs_lookup(parent_path);
        if(parent && parent->ops && parent->ops->create){
            parent->ops->create(parent, filename, VNODE_TYPE_FILE, &node);
        }            
    }
    if(!node) return NULL;

    if(node->ops && node->ops->open){
        if(node->ops->open(node, flags) < 0) return NULL;
    }

    file_t *file = (file_t *)kmalloc(sizeof(file_t));
    if(!file) return NULL;

    file->vnode = node;
    file->pos = 0;
    file->flags = flags;
    file->ref_count = 1;

    return file;
}

int64_t vfs_read(file_t *file, void *buf, size_t count){
    if(!file || !file->vnode || !file->vnode->ops || !file->vnode->ops->read) return -1;

    int64_t bytes_read = file->vnode->ops->read(file->vnode, file->pos, buf, count);
    if(bytes_read > 0){
        file->pos +=bytes_read;
    }
    return bytes_read;
}

int64_t vfs_write(file_t *file, const void *buf, size_t count){
    if(!file || !file->vnode || !file->vnode->ops || !file->vnode->ops->write) return -1;

    int64_t bytes_written = file->vnode->ops->write(file->vnode, file->pos, buf, count);
    if(bytes_written > 0){
        file->pos += bytes_written;
    }
    return bytes_written;
}

int vfs_close(file_t *file){
    if(!file) return -1;

    file->ref_count--;
    if(file->ref_count <= 0){
        if(file->vnode && file->vnode->ops && file->vnode->ops->close){
            file->vnode->ops->close(file->vnode);
        }
        kfree(file);
    }
    return 0;
}

vnode_t* vfs_get_root(void){
    return root_vnode;
}

void vfs_init(void){

    root_vnode = vfs_create_vnode("", VNODE_TYPE_DIR, NULL);
    uart_puts("[VFS] Virtual File System Infrastructured Initialized.\n");
}