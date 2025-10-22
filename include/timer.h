#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint16_t elapsed_time;

void timer_init(void);

#endif
