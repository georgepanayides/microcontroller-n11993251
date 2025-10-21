#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_start_hz(uint16_t hz);
void buzzer_stop(void);

/* Simon-tone helpers mapping step index 0..3 to fixed notes */
void buzzer_on(uint8_t tone_index);
void buzzer_off(void);

/* Octave controls */
void increase_octave(void);
void decrease_octave(void);


#endif
