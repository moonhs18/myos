#include "ulib.h"
#include "syscall.h"
#include <stdarg.h>

int u_strlen(const char *s){
    int len = 0;
    while (s && s[len]) len++;
    return len;   
}

int u_strcmp(const char *s1, const char *s2){
    while (*s1 && (*s1 == *s2))
    {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int u_strncmp(const char *s1, const char *s2, size_t n){
    while (n && *s1 && (*s1 == *s2))
    {
        s1++; s2++; n--;
    }
    if(n==0) return 0;
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *u_strcpy(char *dest, const char *src){
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

int u_atoi(const char *s){
    int res = 0, sign =1;
    while (*s == ' ') s++;
    if(*s == '-') { sign = -1; s++;}
    else if (*s == '+'){ s++; }
    while(*s >= '0' && *s <='9'){
        res = res*10 + (*s - '0');
        s++;
    }
    return res * sign;
}

void u_memset(void *ptr, int val, size_t n){
    uint8_t *p = (uint8_t *)ptr;
    while (n--) *p++ = (uint8_t)val;   
}

void u_memcpy(void *dest, const void *src, size_t n){
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;   
}

void u_putchar(char c){
    sys_write(1, &c, 1);
}

void u_puts(const char *s){
    sys_write(1, s, u_strlen(s));
    sys_write(1, "\n", 1);
}

static void u_put_hex(uint64_t val){
    const char hex[] = "0123456789ABCDEF";
    char buf[19] = "0x0000000000000000";
    for(int i = 17; i>=2;i--){
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    sys_write(1, buf, 18);
}

static void u_put_dec(int val){
    if(val < 0){
        u_putchar('-');
        val = -val;
    }
    if(val == 0){
        u_putchar('0');
        return;
    }
    char buf[12];
    int idx = 0;
    while(val > 0){
        buf[idx++] = '0' + (val%10);
        val /=10;
    }
    for(int i=idx-1 ; i>=0;i--){
        u_putchar(buf[i]);
    }
}

void u_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    while(*fmt){
        if(*fmt == '%'){
            fmt++;
            if(*fmt == 's'){
                const char *s = va_arg(args, const char *);
                sys_write(1, s ? s : "(null)", u_strlen(s ? s : "(null)"));
            } else if (*fmt == 'd'){
                u_put_dec(va_arg(args, int));
            } else if (*fmt == 'x'){
                u_put_hex(va_arg(args, uint64_t));
            } else if (*fmt == 'c'){
                char c = (char)va_arg(args, int);
                u_putchar(c);
            } else if (*fmt == '%'){
                u_putchar('%');
            }
        } 
        else {
            u_putchar(*fmt);
        }
        fmt++;
    }
    va_end(args);
}


char u_getchar(void){
    char c = 0;
    while(sys_read(0, &c, 1) <= 0){
        sys_yield();
    }
    return c;
}

int u_readline(char *buf, size_t max_len){
    size_t idx = 0;
    while(idx < max_len - 1){
        char c = u_getchar();
        if(c =='\r' || c == '\n'){
            u_puts("");
            break;
        }
        else if (c == '\b' || c == 0x7F)
        {
            if(idx > 0){
                idx--;
                u_putchar('\b');
                u_putchar(' ');
                u_putchar('\b');
            }
        }
        else{
            buf[idx++] = c;
            u_putchar(c);
        }
    }
    buf[idx] = '\0';
    return (int)idx;
}