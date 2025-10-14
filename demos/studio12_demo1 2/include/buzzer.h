#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void play_tone(uint8_t tone);
void stop_tone(void);

void increase_octave();
void decrease_octave();

#endif