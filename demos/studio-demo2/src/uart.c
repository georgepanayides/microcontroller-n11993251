#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"

#define BUFF_SIZE 128

static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);

static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

volatile uint8_t rx_buff[BUFF_SIZE];
volatile uint8_t buff_idx = 0;

void uart_init(void) {

    PORTB.DIRSET = PIN2_bm;    //enable PB2 as an output

    USART0.BAUD = 1389;        // 9600 BAUD @ 3.33 MHz
//    USART0.CTRLA = USART_RXCIE_bm | USART_DREIE_bm;   // enable DRE / RX interrupts

    USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;  //enable Tx/Rx

    stdout = &stdio;
    stdin = &stdio;
}//uart_init

uint8_t serial_bytes_available(void)
{
    return buff_idx;
}

uint8_t uart_getc(void)
{
    while (buff_idx == 0);
    return rx_buff[--buff_idx];
}//uart_getc

void uart_putc(uint8_t c) {
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for TXDATA empty
    USART0.TXDATAL = c;
}//uart_putc

static int stdio_putchar(char c, FILE *stream)
{
    uart_putc(c);
    return c; // the putchar function must return the character written to the stream
}

static int stdio_getchar(FILE *stream)
{
    return uart_getc();
}

// Interrupt places received chars in rx_buff
ISR(USART0_RXC_vect)
{
    rx_buff[buff_idx] = USART0.RXDATAL;
    if (buff_idx < BUFF_SIZE)
        buff_idx++;
}//USART0_RXC_vect
