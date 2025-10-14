#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"
//#include "buzzer.h"

static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);

static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

void uart_init()
{
    PORTB.DIRSET = PIN2_bm; // enable PB2 as an output (UART TX)

    USART0.BAUD = 1389;                           // 9600 BAUD @ 3.33MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // enable TX/RX

    //USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt

    stdout = &stdio;
    stdin = &stdio;
}//uart_init

uint8_t uart_getc(void)
{
     while (!(USART0.STATUS & USART_RXCIF_bm));   //wait for receive complete
     return USART0.RXDATAL;
}

void uart_putc(uint8_t c)
{
    while (!(USART0.STATUS & USART_DREIF_bm)); // wait for data register empty
    USART0.TXDATAL = c;
}

static int stdio_putchar(char c, FILE *stream)
{
    uart_putc(c);
    return c; // the putchar function must return the character written to the stream
}

static int stdio_getchar(FILE *stream)
{
    return uart_getc();
}