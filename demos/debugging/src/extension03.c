/** CODE: Include any header files and macro definitions here. */
#include <stdio.h>
#include <stdint.h>
#include "qutyio.h"

/** EX: E3.0

In this week's tutorial, we used some functions from the libraries
qutyio.h and stdio.h to output data via the serial interface. This can
be a useful tool for debugging your program.

To use the serial interface you first need to initialise the required
hardware on the ATtiny1626, and, if you wish to use certain functions
from the stdio.h library, also redirect the standard input (stdin) and
standard output (stdout) streams to the serial interface.

This has been done for you in the qutyio.h library, through the function
"serial_init".

In this exercise, we will use the "printf" function to output formatted
strings to stdout. The syntax for "printf" is:

    printf(<format string> [, arguments]...);

where <format string> is a string that may contain format specifiers
that are replaced by the values of one or more [arguments]. If no format
specifiers are present in this string, no additional arguments are
required in this function call. Some examples are provided below:

    printf("CAB202")  - prints the string "CAB202" to stdout.
    printf(" ")       - prints a space to stdout.
    printf("%02X", x) - prints the 8-bit integer "x" to stdout,
                        formatted as two uppercase hexadecimal digits,
                        with 0 padding.
    printf("%u", x)   - prints the 16-bit integer "x" to stdout, encoded
                        as an unsigned decimal number. Prefix 'u' with
                        'l' to print an unsigned 32-bit integer.
    printf("\n")      - prints a line feed character to stdout.

Note that the format string can consist of multiple parts, for example:

    uint16_t x = 0xAB;
    printf("x: 0x%03X = %lu\n", x, x);

prints:

(START)x: 0x0AB = 171
(END)

TASK: Write C code that performs the following steps:

1. Include qutyio.h and stdio.h at the top of this file, and initialise
   the serial interface using the "serial_init" function.
2. Modify the value of the macro "INITIAL_STATE" to your student number
   in decimal.
3. Create a variable "state" and initialise it using the macro defined
   in step 2.
   You should use the smallest fixed-width integer type from the
   stdint.h header file that can hold this value.
4. Iterate through all the numbers from 0 to 255 in sequence, and for
   each number, perform the following steps:
   a. Take the bitwise XOR of the number with the variable "state",
      storing the result back into "state".
   b. Rotate right the bits in "state" at least one time, and until the
      LSB of "state" is a zero. If there are no cleared bits in "state",
      do nothing.
   c. Print the least significant two bytes of "state" to stdout as four
      uppercase hexadecimal digits, followed by a space. The prefix "0x"
      is not required.
   d. Inspect bits 11-4 of "state" (where bit 0 is the LSB), and:
      - if the most significant nibble of this byte is equal to the
        second last digit of your student number, print the word "foo"
        to stdout.
      - if the least significant nibble of this byte is equal to the
        final digit of your student number print the word "bar" to
        stdout.
      - if both of the above conditions are satisfied, print "foobar".
   e. Print a line feed character to stdout.

After step 4, your program should have printed 256 lines to stdout.

EXAMPLES:

Assuming the student number n12345678, if after step 4b "state" holds:

- 0xE00B_C614, the program should print: "C614 \n"
- 0xD76F_77F0, the program should print: "77F0 foo\n"
- 0x7802_F184, the program should print: "F184 bar\n"
- 0xAFB6_F784, the program should print: "F784 foobar\n"

after step 4e.
*/

#ifndef INITIAL_STATE // if INITIAL_STATE is not already defined
/** CODE: Write your code for Ex E3.0 step 2 below this line. */
#define INITIAL_STATE 11993251
/** CODE: Write your code for Ex E3.0 step 2 above this line. */
#endif

int main(void)
{
    /** CODE: Write your code for Ex E3.0 below this line. */

    serial_init(); // call the init function 

    uint32_t state = INITIAL_STATE; // set the state 

    uint8_t last_digit = INITIAL_STATE % 10;
    uint8_t second_last_digit = (INITIAL_STATE / 10) % 10;

    for (uint16_t i = 0; i <= 255; i++)
    {
        state = state ^ i; // using XOR 

        // rotate right until LSB == 0
        if (state != 0xFFFFFFFF)
        {
            do
            {
                uint32_t lsb = state & 1;
                state = (state >> 1) | (lsb << 31);
            } while ((state & 1) == 1);
        }

        // print bottom 16 bits in uppercase hex
        printf("%04X ", (uint16_t)state);

        uint8_t mid_byte = (state >> 4) & 0xFF;
        uint8_t ms_nibble = (mid_byte >> 4) & 0xF;
        uint8_t ls_nibble = mid_byte & 0xF;

        if (ms_nibble == second_last_digit && ls_nibble == last_digit)
        {
            printf("foobar");
        }
        else if (ms_nibble == second_last_digit)
        {
            printf("foo");
        }
        else if (ls_nibble == last_digit)
        {
            printf("bar");
        }

        // print a newline
        printf("\n");
    }

    /** CODE: Write your code for Ex E3.0 above this line. */

    // END OF EXTENSION03 EXERCISES //
    // DO NOT EDIT BELOW THIS LINE  //

    while (1)
        ; // Loop indefinitely
}
