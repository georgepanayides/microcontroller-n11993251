#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

/* Initialise PA4..PA7 with pull-ups and start a 5 ms TCB1 debouncer. */
void   buttons_init(void);

/* Pop next pressed button (S1..S4 => 0..3), or -1 if none pending. */
int8_t buttons_pop(void);

/* True (non-zero) while the given button is physically held down. idx=0..3 */
uint8_t buttons_is_down(uint8_t idx);

/* Clear the pending press FIFO. */
void   buttons_clear_all(void);

/* Inject a synthetic press into the FIFO (used by UART command layer). */
void   buttons_inject(uint8_t idx);

/* Provided so any code (old/new) that calls buttons_enqueue links cleanly. */
void   buttons_enqueue(uint8_t idx);

#endif
