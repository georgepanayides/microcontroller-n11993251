#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t elapsed_time = 0;

void tcb0_init_1ms(void) {
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.CCMP = 3333;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
}

ISR(TCB0_INT_vect) {
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}
