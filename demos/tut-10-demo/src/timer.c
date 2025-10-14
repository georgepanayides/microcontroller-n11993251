#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "display.h"

void timer_init(void)
{
    cli();
    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB1.CCMP = 16667;               // Set interval for 5 ms (16667 clocks @ 3.333 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;      // Enable
    sei();
}

/** EX: 10.3

The function "timer_init" configures TCB1 to generate an interrupt every
5 ms. As only one digit can be displayed at a time, we can multiplex the
display by rapidly switching between the left and right digits. This
gives the illusion that both digits are illuminated, with a refresh rate
of 100 Hz:

    f = 1 / T = 1 / (2 * 0.005 s) = 100 Hz.

TASK: Complete the ISR declared below to implement multiplexing on the
7-segment display. The ISR should transmit the external variables
"left_byte" and "right_byte", included by "display.h", via SPI0.
*/

ISR(TCB1_INT_vect)
{
    /** CODE: Write your code for Ex 10.3 within this ISR. */
    static uint8_t which = 0;

    if (which) {
        SPI0.DATA = left_byte;
    } else {
        SPI0.DATA = right_byte;
    }
    which ^= 1;

    TCB1.INTFLAGS = TCB_CAPT_bm;
}
