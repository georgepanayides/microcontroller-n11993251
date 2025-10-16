#include "buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Simple debounced state - like the demos */
static volatile uint8_t debounced_state = 0xFF;

/* Simple debouncing - call this regularly from main loop */
void buttons_debounce(void) {
    static uint8_t vcount1 = 0;      // vertical counter MSB
    static uint8_t vcount0 = 0;      // vertical counter LSB
     
    uint8_t sample = PORTA.IN;
    uint8_t changed = sample ^ debounced_state;

    // increment vertical counter
    vcount1 = (vcount1 ^ vcount0) & changed;  // update MSB of vertical counter
    vcount0 = ~vcount0 & changed;             // update LSB of vertical counter

    debounced_state ^= (vcount0 & vcount1);   // update debounced when vertical counter = 11
}

/* Get debounced button state for edge detection */
uint8_t buttons_get_debounced_state(void) {
    return debounced_state;
}

/* Simple init - just enable pullups and setup display timer */
void buttons_init(void)
{
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
    
    debounced_state = PORTA.IN;  // Initialize with current state
    
    /* TCB1: 5ms periodic for display only */
    TCB1.CTRLA   = 0;
    TCB1.CNT     = 0;
    TCB1.CCMP    = 16667;
    TCB1.CTRLB   = TCB_CNTMODE_INT_gc;
    TCB1.INTFLAGS = TCB_CAPT_bm;
    TCB1.INTCTRL  = TCB_CAPT_bm;
    TCB1.CTRLA   = TCB_ENABLE_bm;
}

/* Simple 5ms ISR - just for display multiplexing */
ISR(TCB1_INT_vect)
{
    /* Multiplex display every 5 ms */
    extern void display_multiplex(void);
    display_multiplex();

    TCB1.INTFLAGS = TCB_CAPT_bm;
}
