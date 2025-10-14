#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// start released
volatile uint8_t pb_debounced_state = 0xFF;

// assume 3.333 mhz like the course setup
#ifndef F_CLK_PER
#define F_CLK_PER 3333333UL
#endif

// tcb1 runs clk/2 in periodic interrupt mode
#define TCB1_CLK        (F_CLK_PER / 2UL)
#define TCB1_CMP_MS(ms) ((uint16_t)((TCB1_CLK * (ms)) / 1000UL - 1UL))

void buttons_init(void)
{
    // pa4..pa7 have pullups (s1..s4)
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void timer_init(void)
{
    // 5 ms periodic interrupt on tcb1
    TCB1.CTRLA = 0;                   // off while config
    TCB1.CNT   = 0;
    TCB1.CCMP  = TCB1_CMP_MS(5);      // 5 ms
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;  // periodic
    TCB1.INTFLAGS = TCB_CAPT_bm;      // clear
    TCB1.INTCTRL  = TCB_CAPT_bm;      // irq on
    TCB1.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_DIV2_gc; // start @ clk/2

    sei();                            // make sure isrs are enabled
}

// 2-bit vertical counter debounce, all 4 buttons in parallel
// flips a bit after 4 consistent samples -> ~20 ms at 5 ms/sample
ISR(TCB1_INT_vect)
{
    static uint8_t ct0 = 0;
    static uint8_t ct1 = 0;

    uint8_t sample = PORTA.IN;                     // raw pins
    uint8_t delta  = sample ^ pb_debounced_state;  // bits changing?

    // standard 2-bit counter update (only where delta=1)
    ct0 = ~(ct0 & delta);
    ct1 =  (ct1 ^ ct0) & delta;

    // toggle debounced bits when both counters roll over
    uint8_t toggle = delta & ct0 & ct1;
    pb_debounced_state ^= toggle;

    TCB1.INTFLAGS = TCB_CAPT_bm;                   // clear irq
}
