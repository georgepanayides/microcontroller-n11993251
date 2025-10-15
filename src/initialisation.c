#include "initialisation.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

/* 74HC595 LATCH pin */
#define DISP_LATCH_PORT   PORTA
#define DISP_LATCH_PIN_bm PIN1_bm

/* Route SPI0 to PORTC */
#define USE_SPI0_PORTC_ALT1 1

/* 20 MHz clock initialization */
void clock_init_20mhz(void) {
    ccp_write_io((void *)&CLKCTRL.MCLKCTRLB, 0x00);
}

/* GPIO init: latch as output */
void gpio_init(void) {
    DISP_LATCH_PORT.DIRSET = DISP_LATCH_PIN_bm;
    DISP_LATCH_PORT.OUTSET = DISP_LATCH_PIN_bm;
}

/* SPI0 for 74HC595 */
void spi_init_for_display(void) {
#if USE_SPI0_PORTC_ALT1
    PORTMUX.SPIROUTEA = (PORTMUX.SPIROUTEA & ~0x03) | PORTMUX_SPI0_ALT1_gc;

    PORTC.DIRSET = PIN0_bm | PIN2_bm | PIN3_bm;
    PORTC.DIRCLR = PIN1_bm;
    PORTC.OUTSET = PIN3_bm;
#endif

    SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_PRESC_DIV4_gc;
    SPI0.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm;
    SPI0.INTCTRL = SPI_IE_bm;
}

/* Buzzer / TCA0 setup */
#define BUZZER_PORT     PORTB
#define BUZZER_PIN_bm   PIN5_bm

