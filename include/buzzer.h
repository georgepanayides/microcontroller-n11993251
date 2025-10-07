#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_start_hz(uint16_t hz);
void buzzer_stop(void);


#endif
