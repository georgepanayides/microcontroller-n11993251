#include <avr/io.h>
#include <avr/interrupt.h>

#include "buttons.h"

volatile uint8_t pb_debounced = 0xFF;

void pb_debounce(void) {
    static uint8_t vcount1 = 0;      //vertical counter MSB
    static uint8_t vcount0 = 0;      //vertical counter LSB
     
    uint8_t pb_sample = PORTA.IN;

    uint8_t pb_changed = pb_sample ^ pb_debounced;

    //increment vertical counter
    vcount1 = (vcount1 ^ vcount0) & pb_changed;  //update MSB of vertical counter
    vcount0 = ~vcount0 & pb_changed;             //update LSB of vertical counter

    pb_debounced ^= (vcount0 & vcount1);         //update debounced when vertial counter = 11
}//pb_debounce

void pb_init(void) {
    // already configured as inputs by default

    // enable internal pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;            
}//pb_init

// Wrapper for compatibility
void buttons_init(void) {
    pb_init();
    
    // Setup TCB1 for 5ms periodic interrupt (display multiplex + button debounce)
    TCB1.CTRLA   = 0;
    TCB1.CNT     = 0;
    TCB1.CCMP    = 16667;  // 3.3 MHz / 16667 = ~5ms
    TCB1.CTRLB   = TCB_CNTMODE_INT_gc;
    TCB1.INTFLAGS = TCB_CAPT_bm;
    TCB1.INTCTRL  = TCB_CAPT_bm;
    TCB1.CTRLA   = TCB_ENABLE_bm;
}

// TCB1 ISR: Called every 5ms for display multiplexing and button debouncing
ISR(TCB1_INT_vect)
{
    // Multiplex display
    extern void swap_display_digit(void);
    swap_display_digit();
    
    // Debounce buttons
    pb_debounce();

    TCB1.INTFLAGS = TCB_CAPT_bm;
}


