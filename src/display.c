#include "display.h"
#include "display_macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define DISP_LATCH_PORT   PORTA
#define DISP_LATCH_PIN_bm PIN1_bm

/* Display buffer */
volatile uint8_t digit_l = DISP_OFF;
volatile uint8_t digit_r = DISP_OFF;

static inline void spi0_send(uint8_t b) {
    SPI0.DATA = b;
}

static inline void latch_pulse(void) {
    DISP_LATCH_PORT.OUTCLR = DISP_LATCH_PIN_bm;
    DISP_LATCH_PORT.OUTSET = DISP_LATCH_PIN_bm;
}

/* Compose the byte: [Q7=digit select][Q6..Q0 = segments] */
static inline uint8_t make_byte(uint8_t seg_mask_7bits, uint8_t lhs) {
    uint8_t b = (seg_mask_7bits & 0x7F);
    if (lhs) b |= 0x80;
    return b;
}

void display_init(void) {
    digit_l = DISP_OFF;
    digit_r = DISP_OFF;
}

/* Set display buffer */
void display_set(uint8_t lhs_mask, uint8_t rhs_mask) {
    digit_l = lhs_mask;
    digit_r = rhs_mask;
}

/* Shorthand helpers */
void display_set_lhs(uint8_t mask) {
    digit_l = mask;
}

void display_set_rhs(uint8_t mask) {
    digit_r = mask;
}

void display_set_blank(void) {
    digit_l = DISP_OFF;
    digit_r = DISP_OFF;
}

/* Legacy functions */
void display_write_lhs(uint8_t mask) {
    digit_l = mask;
}

void display_write_rhs(uint8_t mask) {
    digit_r = mask;
}

void display_blank(void) {
    digit_l = DISP_OFF;
    digit_r = DISP_OFF;
}

/* Called by timer ISR to multiplex */
void display_multiplex(void) {
    static uint8_t digit = 0;
    if (digit) {
        spi0_send(make_byte(digit_l, 1));
    } else {
        spi0_send(make_byte(digit_r, 0));
    }
    digit = !digit;
}

/* SPI transfer complete ISR */
ISR(SPI0_INT_vect) {
    DISP_LATCH_PORT.OUTCLR = DISP_LATCH_PIN_bm;
    DISP_LATCH_PORT.OUTSET = DISP_LATCH_PIN_bm;
    SPI0.INTFLAGS = SPI_IF_bm;
}

