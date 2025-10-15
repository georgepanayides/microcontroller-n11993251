#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>

/* Display buffer - holds segment patterns for LHS and RHS */
extern volatile uint8_t digit_l;
extern volatile uint8_t digit_r;

// Basic display functions 
void display_init(void);                              // Initialize SPI and display 
void set_display_segments(uint8_t segs_l, uint8_t segs_r); /* Main display function */
void display_write(uint8_t data);                     /* Send data via SPI */
void swap_display_digit(void);                        /* Multiplex function */

/* Legacy compatibility functions for main.c */
void display_set(uint8_t lhs_mask, uint8_t rhs_mask); /* Wrapper for set_display_segments */
void display_blank(void);                             /* Set display to blank */
void display_multiplex(void);                         /* Called by timer ISR */

#endif
