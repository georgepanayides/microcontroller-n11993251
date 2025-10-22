#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// volatile uint16_t elapsed_time = 0;

// void tcb0_init_1ms(void) {
//     TCB0.CTRLB = TCB_CNTMODE_INT_gc;
//     TCB0.CCMP = 3333;
//     TCB0.INTCTRL = TCB_CAPT_bm;
//     TCB0.CTRLA = TCB_ENABLE_bm;
// }

// ISR(TCB0_INT_vect) {
//     elapsed_time++;
//     TCB0.INTFLAGS = TCB_CAPT_bm;
// }

volatile uint16_t elapsed_time = 0;

void timer_init(void) {
    // configure TCB0 for a periodic interrupt every 1ms
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 3333;                   // Set interval for 1ms (3333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable TCB0
    
    // Note: TCB1 is handled by buttons.c for debouncing and display multiplexing
}

// periodic interrupt every 1ms
ISR(TCB0_INT_vect) { 
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}
