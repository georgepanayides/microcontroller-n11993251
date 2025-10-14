#include "display.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>

/** Tutorial 10

INTRODUCTION:

In this week's tutorial you will work with the USART0 and SPI0
peripherals to read/write data via the USB-UART interface and control
the 7-segment display.
*/

/** EX: 10.0

The following functions have been defined in "uart.c" and included from
"uart.h":

    void uart_init(void);   // Initialise the UART peripheral to 9600 baud, 8N1
    char uart_getc(void);   // Blocking read of byte c from UART
    void uart_putc(char c); // Blocking write of byte c to UART

TASK: Implement the function "uart_puts" in "uart.c" which returns no
result, takes as an argument a pointer to a character (string), and
transmits each character in this string via UART using "uart_putc".
*/

/** CODE: Write your code for Ex 10.0 in uart.c. */

#define SEG_ONE 0x6B  // Q7 = digit-select


int main(void)
{
    timer_init();
    uart_init();

    /** EX: 10.1

    TASK: Write code below that will call "uart_puts" to print your
    student number via the UART interface.

    The string should begin with a leading 'n' and should be terminated
    with a line feed character '\n'.
    */

    uart_puts("n11993251\n");


    /** CODE: Write your code for Ex 10.1 above this line. */

    /** EX: 10.2

    See src/display.c.
    */

    /** TODO: Uncomment the line below after Ex 10.2 is completed. */
    spi_init(); // Initialise SPI

    /** EX: 10.3

    See src/timer.c.
    */

    /** EX: 10.4

    TASK: Write code below that will display the first and second digits
    of your student number, after the character 'a' has been received
    via the UART interface.

    Use the "update_display" function to update the values of
    "left_byte" and "right_byte" with the appropriate segments for the
    first and second digits of your student number.

    HINT: The segments corresponding to digits 0-9 are outlined in the
    truth table below. See the QUTy schematic to determine the pin to
    segment mapping.

       ABCDEFG
    0: 0000001
    1: 1001111
    2: 0010010
    3: 0000110
    4: 1001100
    5: 0100100
    6: 0100000
    7: 0001111
    8: 0000000
    9: 0000100
    */


    /** CODE: Write your code for Ex 10.4 above this line. */

    while (1) {
        char c = uart_getc();
        if (c == 'a' || c == 'A') {
            // Show first two digits of your student number: 1 and 1
            update_display(SEG_ONE, SEG_ONE);
        }
    }
}
