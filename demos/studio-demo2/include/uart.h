#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);

uint8_t uart_getc(void); 
void uart_putc(uint8_t c);

uint8_t serial_bytes_available(void); // Returns number of bytes in receive buffer

#endif