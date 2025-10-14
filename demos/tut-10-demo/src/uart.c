#include "uart.h"

#include <avr/io.h>
#include <stdint.h>

void uart_init(void)
{
    // Output enable USART0 TXD (PB2)
    PORTB.DIRSET = PIN2_bm;

    USART0.BAUD = 1389;                           // 9600 baud @ 3.333 MHz
    // 8 data, no parity, 1 stop (8N1), async
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
                 | USART_PMODE_DISABLED_gc
                 | USART_SBMODE_1BIT_gc
                 | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Enable Tx/Rx
    
}

char uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
        ; // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
        ; // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

/** CODE: Write your code for Ex 10.0 below this line. */

void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

/** CODE: Write your code for Ex 10.0 above this line. */
