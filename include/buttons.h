#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

/* Initialise PA4..PA7 with pull-ups */
void   buttons_init(void);

/* Simple debouncing - call regularly from main loop */
void   buttons_debounce(void);

/* Get debounced button state for edge detection */
uint8_t buttons_get_debounced_state(void);

#endif
