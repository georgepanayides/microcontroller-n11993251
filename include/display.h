#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>

/* Display buffer - holds segment patterns for LHS and RHS */
extern volatile uint8_t digit_l;
extern volatile uint8_t digit_r;

/* Initialise display (SPI already configured elsewhere) */
void display_init(void);

/* Set display buffer (updated by multiplex ISR) */
void display_set(uint8_t lhs_mask, uint8_t rhs_mask);
void display_set_lhs(uint8_t mask);
void display_set_rhs(uint8_t mask);
void display_set_blank(void);

/* Legacy functions (now update buffer instead of writing directly) */
void display_write_lhs(uint8_t mask);
void display_write_rhs(uint8_t mask);
void display_blank(void);

/* Called by timer ISR to multiplex display */
void display_multiplex(void);


#endif
