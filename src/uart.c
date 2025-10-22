#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include "uart.h"
#include "buzzer.h"

volatile int8_t uart_game_input = -1;
volatile uint8_t uart_input_enabled = 0;

static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);
static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

void uart_init()
{
    PORTB.DIRSET = PIN2_bm;
    USART0.BAUD = 1389;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
    USART0.CTRLA = USART_RXCIE_bm;
    stdout = &stdio;
    stdin = &stdio;
}

ISR(USART0_RXC_vect)
{
    uint8_t rx = USART0.RXDATAL;
    
    // Always handle octave changes (INC FREQ / DEC FREQ)
    if (rx == ',' || rx == 'k') {
        increase_octave();
        return;
    }
    if (rx == '.' || rx == 'l') {
        decrease_octave();
        return;
    }
    
    // Only process game inputs when enabled
    if (!uart_input_enabled) return;
    
    // Only accept valid game inputs - ignore everything else
    if (rx == '1' || rx == 'q') uart_game_input = 0;
    else if (rx == '2' || rx == 'w') uart_game_input = 1;
    else if (rx == '3' || rx == 'e') uart_game_input = 2;
    else if (rx == '4' || rx == 'r') uart_game_input = 3;
    // Invalid characters are automatically discarded - no blocking!
}

uint8_t uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm));
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c)
{
    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = c;
}

static int stdio_putchar(char c, FILE *stream)
{
    uart_putc(c);
    return c;
}

static int stdio_getchar(FILE *stream)
{
    return uart_getc();
}
