#include "buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#  define F_CPU 3333333UL
#endif

/* Logical indices 0..3 map to S1..S4 on PA4..PA7 */
static inline uint8_t mask_for_idx(uint8_t idx) {
    static const uint8_t m[4] = { PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };
    return (idx < 4) ? m[idx] : 0;
}

/* Debouncer state */
static volatile uint8_t debounced_state = (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);
static uint8_t diff_count[4];

/* True while physically down */
uint8_t buttons_is_down(uint8_t idx) {
    uint8_t m = mask_for_idx(idx);
    return (m && ((debounced_state & m) == 0)) ? 1u : 0u;
}

/* Get raw debounced button state (for edge detection) */
uint8_t buttons_get_debounced_state(void) {
    return debounced_state;
}

/* Init: PA4..PA7 pull-ups, TCB1 periodic @ 5 ms */
void buttons_init(void)
{
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

    debounced_state = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

     /* TCB1: 5ms periodic */
     TCB1.CTRLA   = 0;
     TCB1.CNT     = 0;
     TCB1.CCMP    = 16667;
     TCB1.CTRLB   = TCB_CNTMODE_INT_gc;
     TCB1.INTFLAGS = TCB_CAPT_bm;
     TCB1.INTCTRL  = TCB_CAPT_bm;
     TCB1.CTRLA   = TCB_ENABLE_bm;
}

/* 5 ms debouncer ISR */
ISR(TCB1_INT_vect)
{
    uint8_t sample = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

    /* For each button, debounce based on 3 consecutive samples */
    for (uint8_t idx = 0; idx < 4; idx++) {
        uint8_t mask = mask_for_idx(idx);
        uint8_t raw_bit = (sample & mask);
        uint8_t deb_bit = (debounced_state & mask);

        if ((raw_bit != 0) != (deb_bit != 0)) {
            if (diff_count[idx] < 3) diff_count[idx]++;
            if (diff_count[idx] >= 3) {
                if (raw_bit == 0) {
                    debounced_state &= (uint8_t)~mask;
                    // No FIFO needed - using edge detection like demos
                } else {
                    debounced_state |= mask;
                }
                diff_count[idx] = 0;
            }
        } else {
            diff_count[idx] = 0;
        }
    }

    /* Multiplex display every 5 ms */
    extern void display_multiplex(void);
    display_multiplex();

    TCB1.INTFLAGS = TCB_CAPT_bm;
}
