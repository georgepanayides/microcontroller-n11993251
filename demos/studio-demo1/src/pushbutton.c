#include <avr/io.h>

#include "pushbutton.h"

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
