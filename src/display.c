
#include <avr/io.h>
#include <avr/interrupt.h>

#include "display.h"
#include "display_macros.h"

volatile uint8_t digit_l = SEGS_OFF;
volatile uint8_t digit_r = SEGS_OFF;

uint8_t number_segs [] = {
    SEGS_ZERO, SEGS_ONE, SEGS_TWO, SEGS_THREE, SEGS_FOUR, SEGS_FIVE, SEGS_SIX, SEGS_SEVEN, 
    SEGS_EIGHT, SEGS_NINE, SEGS_A, SEGS_B, SEGS_C, SEGS_D, SEGS_E, SEGS_F 
};

// Digit masks for displaying decimal numbers 0-9
const uint8_t digit_masks[10] = {
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_E & DISP_SEG_F,  // 0
    DISP_SEG_B & DISP_SEG_C,                                                        // 1
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_G & DISP_SEG_E & DISP_SEG_D,                // 2
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_G,                // 3
    DISP_SEG_F & DISP_SEG_G & DISP_SEG_B & DISP_SEG_C,                             // 4
    DISP_SEG_A & DISP_SEG_F & DISP_SEG_G & DISP_SEG_C & DISP_SEG_D,                // 5
    DISP_SEG_A & DISP_SEG_F & DISP_SEG_E & DISP_SEG_D & DISP_SEG_C & DISP_SEG_G,   // 6
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C,                                          // 7
    DISP_ON,                                                                        // 8
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_F & DISP_SEG_G   // 9
};

// Patterns for Simon Says game steps (left digit)
const uint8_t left_patterns[4] = {DISP_BAR_LEFT, DISP_BAR_RIGHT, DISP_OFF, DISP_OFF};

// Patterns for Simon Says game steps (right digit)
const uint8_t right_patterns[4] = {DISP_OFF, DISP_OFF, DISP_BAR_LEFT, DISP_BAR_RIGHT};

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
}//display_init

void find_dec_digits(uint8_t num, uint8_t *hundreds, uint8_t *tens, uint8_t *units) {
    *hundreds = 0;    
    *tens = 0;
    *units = num;
    while (*units > 99) {
        (*hundreds)++;
        (*units) -= 100;
    }
 
    while (*units > 9) {
        (*tens)++;
        (*units) -= 10;
    }

    digit_l = number_segs[*tens];
    digit_r = number_segs[*units];
}//find_dec_digits

void find_hex_digits(uint8_t num, uint8_t* digit1, uint8_t* digit0) {
    *digit1 = (num >> 4) & 0x0F;
    *digit0 = (num & 0x0F);

    digit_l = number_segs[*digit1];
    digit_r = number_segs[*digit0];
}//find_dec_digits

void set_display_segments(uint8_t segs_l, uint8_t segs_r) {
    digit_l = segs_l;
    digit_r = segs_r;
}//set_display_segments

// // Assumes num_l and num_r are in the range 0..15
void set_display_numbers(uint8_t num_l, uint8_t num_r) {
     digit_l = number_segs[num_l & 0x0F];
     digit_r = number_segs[num_r & 0x0F];
}//set_display_segments

void display_write(uint8_t data) {
    SPI0.DATA = data;              // Note DATA register used for both Tx and Rx
}//display_write

void swap_display_digit(void) {
    static int digit = 0;
    if (digit) {
        display_write(digit_l | (0x01 << 7));
    } else {
        display_write(digit_r);
    }
    digit = !digit;
}//swap_digit

ISR(SPI0_INT_vect){
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  

    SPI0.INTFLAGS = SPI_IF_bm;
}