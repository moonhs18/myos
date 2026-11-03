#include "ulib.h"
#include "syscall.h"
#include <stdarg.h>

UCODE int u_strlen(const char *s){
    int len = 0;
    while (s && s[len]) len++;
    return len;   
}

UCODE int u_strcmp(const char *s1, const char *s2){
    while (*s1 && (*s1 == *s2))
    {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

UCODE int u_strncmp(const char *s1, const char *s2, size_t n){
    while (n && *s1 && (*s1 == *s2))
    {
        s1++; s2++; n--;
    }
    if(n==0) return 0;
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

UCODE char *u_strcpy(char *dest, const char *src){
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

UCODE int u_atoi(const char *s){
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

UCODE void u_memset(void *ptr, int val, size_t n){
    uint8_t *p = (uint8_t *)ptr;
    while (n--) *p++ = (uint8_t)val;   
}

UCODE void u_memcpy(void *dest, const void *src, size_t n){
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;   
}

UCODE void u_putchar(char c){
    sys_write(1, &c, 1);
}

UCODE void u_puts(const char *s){
    static const char nl[] URODATA = "\n";
    if(s){
        sys_write(1, s, u_strlen(s));
    }
    sys_write(1, nl, 1);
}

UCODE static void u_put_hex(uint64_t val){
    static const char hex[] URODATA = "0123456789ABCDEF";
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';

    for(int i = 17; i>=2;i--){
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    buf[18] = '\0';
    sys_write(1, buf, 18);
}

UCODE static void u_put_dec(int val){
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

UCODE void u_printf(const char *fmt, ...){
    static const char null_str[] URODATA = "(null)";
    va_list args;
    va_start(args, fmt);
    while(fmt && *fmt){
        if(*fmt == '%'){
            fmt++;
            if(*fmt == 's'){
                const char *s = va_arg(args, const char *);
                const char *str = s ? s : null_str;
                sys_write(1, str, u_strlen(str));
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


UCODE int u_getchar(void){
    char c = 0;
    
    int64_t result = sys_read(0, &c, 1);

    if(result != 1){
        return -1;
    }
    
    return (unsigned char)c;
}
UCODE int u_readline(char *buf, size_t max_len) {
    if (!buf || max_len == 0) {
        return -1;
    }

    size_t idx = 0;
    buf[0] = '\0';

    while (1) {
        int c = u_getchar();

        if (c < 0) {
            return -1;
        }

        // Enter
        if (c == '\n' || c == '\r') {
            u_putchar('\n');
            break;
        }

        // Backspace
        if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';

                u_putchar('\b');
                u_putchar(' ');
                u_putchar('\b');
            }

            continue;
        }

        // Printable ASCII
        if (c >= 0x20 && c <= 0x7E) {
            if (idx < max_len - 1) {
                buf[idx++] = (char)c;
                buf[idx] = '\0';

                u_putchar((char)c);
            }
        }
    }

    buf[idx] = '\0';

    return (int)idx;
}