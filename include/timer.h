#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void tcb0_init_1ms(void);
uint32_t millis(void);
void delay_ms(uint16_t ms);

#endif
