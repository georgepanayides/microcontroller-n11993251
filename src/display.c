#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "display.h"
#include "display_macros.h"

// Display buffer - like demos
volatile uint8_t digit_l = DISP_OFF;
volatile uint8_t digit_r = DISP_OFF;

void display_init(void) {
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;  // SPI pins on PC0-3

    // SPI SCK and MOSI
    PORTC.DIRSET = (PIN0_bm | PIN2_bm);   // SCK (PC0) and MOSI (PC2) output

    // DISP_LATCH
    PORTA.OUTSET = PIN1_bm;        // DISP_LATCH initial high
    PORTA.DIRSET = PIN1_bm;        // set DISP_LATCH pin as output

    SPI0.CTRLA = SPI_MASTER_bm;    // Master, /4 prescaler, MSB first
    SPI0.CTRLB = SPI_SSD_bm;       // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;      // Interrupt enable
    SPI0.CTRLA |= SPI_ENABLE_bm;   // Enable
}

// Main display function
void set_display_segments(uint8_t segs_l, uint8_t segs_r) {
    digit_l = segs_l;
    digit_r = segs_r;
}

void display_write(uint8_t data) {
    SPI0.DATA = data;         
}

// Multiplexing function
void swap_display_digit(void) {
    static int digit = 0;
    if (digit) {
        display_write(digit_l | (0x01 << 7));
    } else {
        display_write(digit_r);
    }
    digit = !digit;
}

void display_set(uint8_t lhs_mask, uint8_t rhs_mask) {
    set_display_segments(lhs_mask, rhs_mask);
}

void display_blank(void) {
    set_display_segments(DISP_OFF, DISP_OFF);
}

void display_multiplex(void) {
    swap_display_digit();
}

ISR(SPI0_INT_vect) {
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  

    SPI0.INTFLAGS = SPI_IF_bm;
}

