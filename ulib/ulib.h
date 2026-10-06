#ifndef ULIB_H
#define ULIB_H

#include <stdint.h>
#include <stddef.h>

int     u_strlen(const char *s);
int     u_strcmp(const char *s1, const char *s2);
int     u_strncmp(const char *s1, const char *s2, size_t n);
char   *u_strcpy(char *dest, const char *src);
int     u_atoi(const char *s);
void    u_memset(void *ptr, int val, size_t n);
void    u_memcpy(void *dest, const void *src, size_t n);

void    u_putchar(char c);
void    u_puts(const char *s);
void    u_printf(const char *fmt, ...);
char    u_getchar(void);
int     u_readline(char *buf, size_t max_len);

#endif