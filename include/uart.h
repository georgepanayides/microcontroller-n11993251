#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();                    // Initialise UART as stdin/stdout

uint8_t uart_getc (void);
void uart_putc(uint8_t c);

// Global volatile variable for UART game input (set by ISR)
extern volatile int8_t uart_game_input;

// Simple flag: only accept game input during user input phase
extern volatile uint8_t uart_input_enabled;

#endif
