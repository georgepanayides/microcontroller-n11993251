#include "display.h"
#include "display_macros.h"
#include <avr/io.h>

/* Must match initialisation.c latch macros */
#define DISP_LATCH_PORT   PORTA
#define DISP_LATCH_PIN_bm PIN1_bm

static inline void spi0_send(uint8_t b) {
    SPI0.DATA = b;
    while (!(SPI0.INTFLAGS & SPI_RXCIF_bm)) { }
    (void)SPI0.DATA; /* clear */
}

static inline void latch_pulse(void) {
    DISP_LATCH_PORT.OUTSET = DISP_LATCH_PIN_bm;
    DISP_LATCH_PORT.OUTCLR = DISP_LATCH_PIN_bm;
}

/* Compose the byte: [Q7=digit select][Q6..Q0 = active-low segments] */
static inline uint8_t make_byte(uint8_t seg_mask_7bits, uint8_t lhs) {
    uint8_t b = (seg_mask_7bits & 0x7F);
    if (lhs) b |= 0x80; /* Q7=1 -> LHS */
    return b;
}

void display_init(void) {
    /* Start blank (RHS off is fine as a default state) */
    spi0_send(make_byte(DISP_OFF, 0));  /* RHS off */
    latch_pulse();
}

void display_write_lhs(uint8_t mask) {
    spi0_send(make_byte(mask, 1));
    latch_pulse();
}

void display_write_rhs(uint8_t mask) {
    spi0_send(make_byte(mask, 0));
    latch_pulse();
}

void display_blank(void) {
    /* Blank the currently selected side; for our usage we blank RHS by default */
    spi0_send(make_byte(DISP_OFF, 0));
    latch_pulse();
}
