#include "display.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

/** EX: 10.2.0

Only 7 bits are used to control the 7-segment display, defining the
segment pattern. The remaining bit determines which digit is being
driven.

TASK: Refer to the QUTy schematic to identify which bit serves this
purpose and update the "DISP_OFF" and "DISP_LHS" macros below.

- "DISP_OFF" represents a segment pattern where all 7 segments are off.
- "DISP_LHS" is a bitmask that enables the left-hand side (LHS) digit.
  Applying "DISP_LHS" to a segment pattern should place it on the LHS
  digit.
*/

/** CODE: Write your code for Ex 10.2.0 below this line. */
#define DISP_OFF 0x7F // all segments OFF
#define DISP_LHS 0x80 // Q7 bit
/** CODE: Write your code for Ex 10.2.0 above this line. */

// Display initially blank
volatile uint8_t left_byte = DISP_OFF | DISP_LHS;
volatile uint8_t right_byte = DISP_OFF;

/** EX: 10.2.1

TASK: Declare and implement the function "spi_init" that will initialise
SPI0 in unbuffered mode, such that data can be written to the shift
register that controls the 7-segment display.

This function should also enable the SPI interrupt via the IE bit in the
INTCTRL register.

Once this exercise is complete, add a function prototype for "spi_init"
in "display.h".
*/

/** CODE: Write your code for Ex 10.2.1 below this line. */

void spi_init(void) {
    // Route SPI0 to PC0..PC3 (ALT1); SCK=PC0, MOSI=PC2
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;
    PORTC.DIRSET = PIN0_bm | PIN2_bm; // SCK, MOSI outputs

    // LATCH on PA1 
    PORTA.OUTCLR = PIN1_bm;
    PORTA.DIRSET = PIN1_bm;

    // SPI master, MSB first, mode 0, unbuffered
    SPI0.CTRLA   = SPI_MASTER_bm; 
    SPI0.CTRLB   = SPI_SSD_bm;              
    SPI0.INTCTRL = SPI_IE_bm;              
    SPI0.CTRLA  |= SPI_ENABLE_bm; // enable
}

/** CODE: Write your code for Ex 10.2.1 above this line. */

/** EX: 10.2.2

When 8 bits are transmitted via SPI0, we must latch the output of the
shift register to update the state of the 7-segment display. We can use
an interrupt to detect when a transmission is complete, before latching
the output from within the ISR.

TASK: Implement the SPI0 ISR to latch the output of the shift register
when a transmission is complete. The ISR should also clear the IF bit
in the INTFLAGS register.

Once this exercise is complete, uncomment the line in "tutorial10.c"
that calls "spi_init". The 7-segment display should be blank after this
function is called.
*/

/** CODE: Write your code for Ex 10.2.2 below this line. */

// pulse HIGH then LOW (rising edge triggers storage)

ISR(SPI0_INT_vect) {
    PORTA.OUTSET = PIN1_bm;  // HIGH
    PORTA.OUTCLR = PIN1_bm;  // LOW
    SPI0.INTFLAGS = SPI_IF_bm;
}

/** CODE: Write your code for Ex 10.2.2 above this line. */

/** EX: 10.2.3

The global variables "left_byte" and "right_byte" store the bytes to be
transmitted to the shift register via SPI0. Each byte represents the
state of a digit on the 7-segment display, including the bit which
determines which digit is being driven.

TASK: Implement the function "update_display" so that it updates the
values of "left_byte" and "right_byte" using the arguments "left" and
"right".
*/

void update_display(const uint8_t left, const uint8_t right)
{
    /** CODE: Write your code for Ex 10.2.3 within this function. */
    left_byte  = (left  & 0x7F) | DISP_LHS; // Q7=1 => LHS
    right_byte = (right & 0x7F); // Q7=0 => RHS

}
