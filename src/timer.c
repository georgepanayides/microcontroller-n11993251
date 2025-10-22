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

    // configure TCB1 for a periodic interrupt every 5ms
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB1 in periodic interrupt mode
    TCB1.CCMP = 16667;                  // Set interval for 5ms (16667 clocks @ 3.3 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;         // Enable TCB1

}//timer_init

// periodic interrupt every 1ms
ISR(TCB0_INT_vect) { 
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}//TCB0_INT_vect

// periodic interrupt every 5ms
ISR(TCB1_INT_vect) { 
    pb_debounce();
    TCB1.INTFLAGS = TCB_CAPT_bm;
}//TCB1_INT_vect
