#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART0_BASE 0x09000000

#define UARTDR      ((volatile uint32_t *)(UART0_BASE + 0x00)) //Data Register for rx tx
#define UARTFR      ((volatile uint32_t *)(UART0_BASE + 0x18)) //Flag Register for state flag
#define UARTIBRD    ((volatile uint32_t *)(UART0_BASE + 0x24)) //Interger Baud Rate Divisior
#define UARTFBRD    ((volatile uint32_t *)(UART0_BASE + 0x28)) //Fractional Baud Rate Divisior
#define UARTLCR_H   ((volatile uint32_t *)(UART0_BASE + 0x2C)) //Line Control Register
#define UARTCR      ((volatile uint32_t *)(UART0_BASE + 0x30)) //Control Register

#define FR_TXFF (1<<5) //Transmit FIFO Full
#define FR_RXFE (1<<4) //Receive FIFO Empty

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_hex(uint64_t val);

#endif