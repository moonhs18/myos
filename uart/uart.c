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
    while ()
    
}