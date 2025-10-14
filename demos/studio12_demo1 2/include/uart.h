#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();                    // Initialise UART as stdin/stdout

uint8_t uart_getc (void);
void uart_putc(uint8_t c);   

#endif