#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "qutyio.h"

/** EX: 7.2

To display the elapsed time on the 7-segment display, we need to count
the number of times the TCB0 CAPT interrupt is invoked.

TASK: Declare a global variable called "count_64" that will be used
to store a count of 1/64 seconds that have elapsed.

Ensure this variable is sufficiently large so that it can represent the
largest value that can be displayed on the 7-segment display.

The stopwatch should initially display 0x00.
*/

volatile uint16_t count_64 = 0;


/** CODE: Write your code for Ex 7.2 above this line. */

/** EX: 7.3


To allow pushbuttons S1 and S2 to control when the stopwatch is
counting, we need to keep track of the state of the stopwatch.

TASK: Declare a global variable called "is_counting" that keeps track
of the stopwatch's state.

The stopwatch should initially be paused.
*/

volatile uint8_t is_counting = 0;


/** CODE: Write your code for Ex 7.3 above this line. */

/** EX: 7.4

TASK: Declare an interrupt service routine (ISR) for the TCB0 peripheral
that was configured in Ex 7.0.

1. Find the vector number and name that correspond to the TCB0 CAPT
   interrupt.
   Refer to datasheet Section 8.2 Interrupt Vector Mapping on p. 63.
2. Given that vectors are of the form "*_vect", find the appropriate
   macro corresponding to the vector number identified in step 1.
   Type "_vect" to invoke VSCode suggestions.
3. Declare an ISR using the ISR(<vector>) macro syntax, where <vector>
   is the macro identified in step 2.
4. If the stopwatch is currently counting, increment "count_64" by 1.
   Ensure that this variable does not exceed the value which represents
   15.9375 s (0xFF * 1/16 s).
5. As the value displayed on the 7-segment display is a count of 1/16 s,
   define a local variable called "count_16" and convert "count_64" to
   a count of 1/16 s.
6. Update the 7-segment display to reflect the value of "count_16".
7. Acknowledge that this interrupt has been handled by clearing the CAPT
   bit in the TCB0.INTFLAGS register.
   See ATtiny1626 Datasheet 22.5.5 Interrupt Flags on p. 260 for
   information on how to clear this bit.

NOTE: You may use the "display_hex" function to display a number in
hexadecimal on the 7-segment display. This function accepts an unsigned
8-bit integer.

To find the maximum value of "count_64" in step 4 and to perform the
conversion in step 5, you will need to use the following relationship to
convert between counts of 1/64 s and counts of 1/16 s:

    1/16 second = 4 * (1/64 second)
*/

ISR(TCB0_INT_vect)
{
    if (is_counting && count_64 < 0x03FC) {
        count_64++;  // 1/64s tick
    }

    uint8_t count_16 = (uint8_t)(count_64 >> 2);  // convert to 1/16 s
    display_hex(count_16);

    // clear the interrupt flags
    TCB0.INTFLAGS = TCB_CAPT_bm;
}


/** CODE: Write your code for Ex 7.4 above this line. */
