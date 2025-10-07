#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>

/* Initialise display (SPI already configured elsewhere) */
void display_init(void);

/* Write one digit at a time. 'mask' is the 7-bit active-low segment mask. */
void display_write_lhs(uint8_t mask);   /* Q7=1 selects LHS */
void display_write_rhs(uint8_t mask);   /* Q7=0 selects RHS */

/* Convenience: force current digit off (writes a single-byte OFF pattern). */
void display_blank(void);

#endif
