// The functions defined in this file can be accessed by including the
// header file associated with this source file (initialisation.h).
//
// It is included here to ensure that the function prototypes defined in
// initialisation.h are consistent with their definitions.
#include "initialisation.h"

#include <avr/io.h>
#include <avr/interrupt.h>

/** EX: 7.0

When configuring hardware peripherals to generate interrupts, it is
important to ensure that interrupts cannot be raised by other
peripherals while this configuration is taking place.

TASK: Disable interrupts globally before configuring the TCB0 peripheral
and re-enable interrupts globally afterwards.

HINT: Use the "cli" and "sei" functions provided by avr/interrupt.h.
*/

void clock_init(void)
{
    /** CODE: Write your code for Ex 7.0 within this function. */
    cli();
    // TCB0 is a 16-bit timer that has been configured to generate
    // an interrupt every 64th of a second. This was done to increase
    // the resolution of the timer.
    TCB0.CCMP = 52083;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
    sei();
}

/** EX: 7.1

The stopwatch will be controlled by pushbuttons S1, S2, and S4.

TASK: Write code to configure the pins connected to these pushbuttons
to generate an interrupt on falling edges.

See ATtiny1626 Datasheet 17.4 Register Summary - PORTx on p. 153 and
17.5.12 Pin n Control on p. 165.
*/

void buttons_init(void)
{
    /** CODE: Write your code for Ex 7.1 within this function. */

    // falling edge and pin up for btn S1 PA4 
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;

    // falling edge and pin up for btn S2 PA5
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;

    // falling edge and pin up for btn S4 PA7
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;

}
