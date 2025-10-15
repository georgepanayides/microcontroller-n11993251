#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

/* Initialise PA4..PA7 with pull-ups and start a 5 ms TCB1 debouncer. */
void   buttons_init(void);

/* True (non-zero) while the given button is physically held down. idx=0..3 */
uint8_t buttons_is_down(uint8_t idx);

/* Get raw debounced button state (for edge detection). Returns PA7-PA4 bits. */
uint8_t buttons_get_debounced_state(void);

#endif
