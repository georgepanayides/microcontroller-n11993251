#include <avr/io.h>
#include <avr/interrupt.h>
#include "qutyio.h"

#include "timer.h"

/** EX: 7.6

TASK: Write an interrupt service routine for PORTA, that will be invoked
when a falling edge is detected on any pins connected to pushbuttons S1,
S2, or S4.

This ISR should check which pin generated the interrupt, and perform
one of the following actions:

- If S1 is pressed, start the stopwatch.
- If S2 is pressed, pause the stopwatch.
- If S4 is pressed, reset the stopwatch to 0x00.

See ATtiny1626 Datasheet 17.5.10 Interrupt Flags on p. 163 for
information on how to clear these interrupt flags.

NOTE: Each pin has its own interrupt flag, and you must only clear the
interrupt flag for the pin that generated the interrupt.
*/

volatile uint16_t count_64;
volatile uint8_t is_counting;


ISR(PORTA_PORT_vect)
{
    uint8_t flags = PORTA.INTFLAGS;

    // start
    if (flags & PIN4_bm) {
        is_counting = 1;
        PORTA.INTFLAGS = PIN4_bm; // clear only PA4
    }

    // pause
    if (flags & PIN5_bm) {
        is_counting = 0;
        PORTA.INTFLAGS = PIN5_bm; // clear only PA5
    }

    // dont change is_counting here
    if (flags & PIN7_bm) {
        count_64 = 0;
        display_hex(0x00); // reset display
        PORTA.INTFLAGS = PIN7_bm; // clear only PA7
    }
}


/** CODE: Write your code for Ex 7.6 above this line. */
