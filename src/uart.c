#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

// External UART button flags from main.c
extern volatile uint8_t uart_s1, uart_s2, uart_s3, uart_s4;

static int stdio_putchar(char c, FILE *stream);
static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, NULL, _FDEV_SETUP_RW);

void uart_init_9600_8n1(void)
{
    PORTB.DIRSET = PIN2_bm; // enable PB2 as an output (UART TX)

    USART0.BAUD = 1389;                           // 9600 BAUD @ 3.33MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // enable TX/RX

    USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt

    stdout = &stdio;
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

ISR(USART0_RXC_vect) 
{
    uint8_t rx_data = USART0.RXDATAL;
    
    // Simple button simulation for Simon Says
    switch (rx_data) {
        case '1': case 'q': uart_s1 = 1; break; /* S1 */
        case '2': case 'w': uart_s2 = 1; break; /* S2 */
        case '3': case 'e': uart_s3 = 1; break; /* S3 */
        case '4': case 'r': uart_s4 = 1; break; /* S4 */
    }
}

/* Simple functions for main.c compatibility */
uint16_t uart_apply_freq_offset(uint16_t base_hz)
{
    return base_hz; // No frequency offset for simplicity
}

uint8_t uart_seed_pending(void) { return 0; }
uint32_t uart_take_seed(void) { return 0; }

void uart_game_success(uint16_t score)
{
    printf("SUCCESS %u\n", score);
}

void uart_game_over(uint16_t score)
{
    printf("GAME OVER %u\n", score);
}
