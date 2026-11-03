#include "uart.h"

void uart_init(void){
    *UARTCR = 0x0;

    /*  Baudrate setting base 115200 - 24MHz
        IBRD = 24000000 / (16 * 115200) = 13.0208 -> 13
        FBRD = 0.0208 * 64 = 1.33 -> 1 */
    *UARTIBRD = 13;
    *UARTFBRD = 1;

    *UARTLCR_H = (1<<4) | (3<<5);

    *UARTCR = (1<<0) | (1<<8) | (1<<9);
}

void uart_putc(char c){
    while (*UARTFR & FR_TXFF);

    if(c=='\n'){
        *UARTDR = '\r';
        while (*UARTFR & FR_TXFF);
    }
    *UARTDR = (uint32_t)c;
}

void uart_puts(const char *s){
    while(*s){
        uart_putc(*s++);
    }
}

void uart_put_hex(uint64_t val){
    const char hex_chars[] = "0123456789ABCDEF";
    uart_puts("0x");
    for(int i = 60; i >= 0; i -= 4){
        uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}


char uart_getc(void)
{
    static int discard_lf_after_cr = 0;

    for (;;) {

        // Wait until RX FIFO contains data.
        while (*UARTFR & FR_RXFE) {
            asm volatile("yield" ::: "memory");
        }

        char c = (char)(*UARTDR & 0xFF);

        // CR -> LF
        if (c == '\r') {
            discard_lf_after_cr = 1;
            return '\n';
        }

        // Discard LF following CR.
        if (c == '\n' && discard_lf_after_cr) {
            discard_lf_after_cr = 0;
            continue;
        }

        discard_lf_after_cr = 0;

        // DEL -> Backspace
        if ((unsigned char)c == 0x7F) {
            c = '\b';
        }

        return c;
    }
}


int uart_has_char(void){
    return !(*UARTFR & FR_RXFE);
}


int uart_try_getc(char *out)
{
    static int discard_lf_after_cr = 0;

    if (out == 0) {
        return -1;
    }

    for (;;) {

        if (*UARTFR & FR_RXFE) {
            return 0;
        }

        char c = (char)(*UARTDR & 0xFF);

        if (c == '\r') {
            discard_lf_after_cr = 1;
            c = '\n';
        }

        else if (c == '\n' && discard_lf_after_cr) {
            discard_lf_after_cr = 0;
            continue;
        }

        else {
            discard_lf_after_cr = 0;
        }

        if ((unsigned char)c == 0x7F) {
            c = '\b';
        }

        *out = c;

        return 1;
    }
}
