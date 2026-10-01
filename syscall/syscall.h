#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "exception.h"

//syscall number
#define SYS_YIELD       0
#define SYS_WRITE       1
#define SYS_GETPID      2
#define SYS_SLEEP       3
#define SYS_EXIT        4
#define SYS_OPEN        5
#define SYS_READ        6
#define SYS_CLOSE       7   
#define MAX_SYSCALL     8   


void syscall_init(void);
void syscall_dispatch(trap_frame_t *tf);

static inline int64_t sys_yield(void){
    register uint64_t x8 asm("x8") = SYS_YIELD;
    register int64_t  x0 asm("x0");
    asm volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
    return x0;
}

static inline int64_t sys_write(int fd, const void *buf, size_t count){
    register uint64_t x8 asm("x8") = SYS_WRITE;
    register int64_t  x0 asm("x0") = (int64_t)fd;
    register uint64_t x1 asm("x1") = (uint64_t)buf;
    register uint64_t x2 asm("x2") = (uint64_t)count;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

static inline int64_t sys_getpid(void){
    register uint64_t x8 asm("x8") = SYS_GETPID;
    register int64_t  x0 asm("x0");
    asm volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
    return x0;
}

static inline int64_t sys_sleep(uint64_t ticks){
    register uint64_t x8 asm("x8") = SYS_SLEEP;
    register int64_t  x0 asm("x0") = (int64_t)ticks;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory");
    return x0;
}

static inline void sys_exit(int code){
    register uint64_t x8 asm("x8") = SYS_EXIT;
    register int64_t  x0 asm("x0") = (int64_t)code;
    asm volatile("svc #0" : : "r"(x8), "r"(x0) : "memory");
    while(1);   
}

static inline int64_t sys_open(const char *path, int flags){
    register uint64_t x8 asm("x8") = SYS_OPEN;
    register uint64_t x0 asm("x0") = (uint64_t)path;
    register uint64_t x1 asm("x1") = (uint64_t)flags;

    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1) : "memory");
    return x0;
}

static inline int64_t sys_read(int fd, void *buf, size_t count){
    register uint64_t x8 asm("x8") = SYS_READ;
    register int64_t  x0 asm("x0") = (int64_t)fd;
    register uint64_t x1 asm("x1") = (uint64_t)buf;
    register uint64_t x2 asm("x2") = (uint64_t)count;

    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

static inline int64_t sys_close(int fd){
    register uint64_t x8 asm("x8") = SYS_CLOSE;
    register int64_t  x0 asm("x0") = (uint64_t)fd;

    asm volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory");
    return x0;
}

#endif