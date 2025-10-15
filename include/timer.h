#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint16_t elapsed_time;

void tcb0_init_1ms(void);

#endif
