#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

/** CODE: Write your code for Ex 7.7 below this line. */

#include "initialisation.h"
#include "timer.h"

/** CODE: Write your code for Ex 7.7 above this line. */
#include "qutyio.h"

/** Tutorial 07

INTRODUCTION:

In this week's tutorial you will implement a stopwatch. This stopwatch
is controlled using the pushbuttons and increments every one sixteenth
of a second. The elapsed time is displayed on the 7-segment display in
hexadecimal sixteenths of a second (i.e., 0x01 represents 0.0625 s).

Pushbutton functionality is described below:

- Pushbutton S1 starts the stopwatch
- Pushbutton S2 pauses the stopwatch
- Pushbutton S4 resets the elapsed time back to 0x00

Once the display reaches 0xFF, it will remain at this value until the
stopwatch is reset.
*/

/** EX: 7.0

See include/initialisation.c.
*/

/** EX: 7.1

See include/initialisation.c.
*/

/** EX: 7.2

See src/timer.c.
*/

/** EX: 7.3

See src/timer.c.
*/

/** EX: 7.4

See src/timer.c.
*/

/** EX: 7.5

See include/timer.h.
*/

/** EX: 7.6

See src/buttons.c.
*/

int main(void)
{
    // Configures pins PA1, PC0, and PC2, to drive the 7-segment display
    display_init();
    // Drives DISP EN net HIGH
    display_on();

    /** EX: 7.7

    TASK: Include the header file containing the initialisation
    functions you wrote in Ex 7.0 and call these functions below.

    Place this preprocessor directive at the top of this file.
    */

    clock_init();
    buttons_init();
    display_hex(0x00);

    /** CODE: Write your code for Ex 7.7 above this line. */

    // This program is interrupt-driven and therefore does not need any
    // additional code in main.
    while (1)
        ; // Loop indefinitely
}
