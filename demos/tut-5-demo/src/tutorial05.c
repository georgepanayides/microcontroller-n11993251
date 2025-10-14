#include <avr/io.h>

#include <stdio.h>

#include "qutyio.h"

// This header file includes code for Exercise 5.5.
#include "preamble.h"

int main(void)
{
    serial_init();

    uint8_t digits;

    /** EX: 5.0

    In the line above, we have defined a variable called "digits" that
    represents an unsigned 8-bit integer.

    TASK: Write C code below to assign a value equal to the last two
    digits of your student number to this variable.

    EXAMPLE: If your student number was n12345678 then you should assign
    the value 78 to "digits".
    */

    digits = 51;

    /** CODE: Write your code for Ex 5.0 above this line. */
    printf("Ex 5.0: digits = %02u\n", digits);

    /** EX: 5.1

    With the help of the definitions included by the "#include <avr/io.h>"
    preprocessor directive above, we can also assign values directly to
    peripheral registers. The register definitions for the ATtiny1626
    are accessed using dot notation.

    EXAMPLE: We could write to the PORTB OUT register using the
    statement: PORTB.OUT = 0;

    TASK: Write C code below to illuminate DS1-DP.

    TIP: Invoke VSCode suggestions using Ctrl + Space (or Command +
    Space on macOS) after typing "PORTB." to view available register
    definitions.
    */

    PORTB.DIRSET = PIN5_bm; // allow PB5 for output
    PORTB.OUTCLR = PIN5_bm; // drive PB5 low to illuminate DP

    /** CODE: Write your code for Ex 5.1 above this line. */
    wait_1ms(); printf("Ex 5.1: expected = 0, observed = %u\n", PORTB_IN & 040);

    /** EX: 5.2

    Below is a partially implemented loop that executes the statement
    "loop += 1" 24 times.

    TASK: Assign a value to the #define preprocessor directive
    "CONDITION" below so that the for loop executes the correct number
    of times.
    */

    /** CODE: Write your code for Ex 5.2 on the following line. */
#define CONDITION (i < 24)

    uint8_t loop = 0;
    for (uint8_t i = 0; CONDITION; i++)
    {
        loop += 1;
    }
    printf("Ex 5.2: iterations = %u\n", loop);

    /** EX: 5.3

    TASK: Write C code below to correctly configure pushbutton S1 by
    enabling the internal pull-up resistor for the corresponding pin.
    */

    // enable the pull up 
    PORTA.DIRCLR = PIN4_bm;
    PORTA.PIN4CTRL |= PORT_PULLUPEN_bm; // enable the pull up resistor 


    /** CODE: Write your code for Ex 5.3 above this line. */
    printf("Ex 5.3: expected = 0xFF, observed = 0x%02X\n", PB ^ 0xF7);

    /** EX: 5.4

    The "while" loop below currently executes forever.

    TASK: Add code to the body of this while loop so that the program
    exits the loop as soon as pushbutton S1 is pressed.
    */

    printf("Ex 5.4: PORTA.IN = 0x%02X. Waiting for pushbutton press...", VPORTA.IN);

    // C uses integers for Boolean values. Any non-zero value represents
    // true and zero represents false.
    while (1) // Loop condition always true
    {
        /** CODE: Write your code for Ex 5.4 within this while loop. */
        if ( (VPORTA.IN & PIN4_bm) == 0 ) {
            break;
        }

    }

    printf("DONE! PORTA.IN = 0x%02X\n", VPORTA.IN);

    /** EX: 5.5

    Elsewhere in the program we have defined two signed 8-bit integers,
    "X" and "Y".

    TASK: Write C code to:

    1. Multiply X by Y and store the result in a variable called "alpha".
    2. If "alpha" is positive:
         perform a bitwise left shift by two positions.
       If "alpha" is negative:
         perform an arithmetic right shift by four positions.
    3. Store the result from step 2 in a variable called "beta".

    Upon completion, uncomment the printf() statement below.
    */

    int16_t alpha = (int16_t)X * (int16_t)Y;
    int16_t beta;
    if (alpha > 0) {
        beta = alpha << 2;
    } else if (alpha < 0) {
        beta = alpha >> 4;
    } else {
        beta = 0;
    }

    /** CODE: Write your code for Ex 5.5 above this line. */

    /** TODO: Uncomment the line below after Ex 5.5 is completed. */
    printf("Ex 5.5: X = %d, Y = %d, alpha = %d, beta = %d\n", X, Y, alpha, beta);

    // END OF TUTORIAL05 EXERCISES //
    // DO NOT EDIT BELOW THIS LINE //

    while (1)
        ; // Loop indefinitely
}
