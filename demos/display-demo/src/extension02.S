#include <avr/io.h> // Include register definitions and useful macros

.section .init0

/** EX: E2.0

U2 is a shift register that controls the 2-digit 7-segment display on
the QUTy. The shift register's outputs, Q0-Q6, drive the LED segments of
the display in an active-low configuration, while output Q7 selects the
LHS or RHS digit via U3 and U4.

The shift register has 8 bits of internal memory which can be set using
the SPI CLK and SPI MOSI nets:

- When the input connected to the SPI CLK net transitions from LOW to
  HIGH (rising edge), bit 0 of the internal memory is set to the current
  state of the SPI MOSI net.

  Simultaneously, the data in the internal memory is shifted by one bit
  position to the left:

  - The previous value in bit 0 is shifted to bit 1.
  - The previous value in bit 1 is shifted to bit 2.
  - ...

  Equivalently:

      internal memory    <- LSL(internal memory)
      internal memory[0] <- MOSI

When the input connected to the DISP LATCH net transitions from LOW to
HIGH (rising edge), the state of this internal memory sets the state
of outputs Q0-Q7:

    Q <- internal memory

For example, if we wanted to turn all segments of the display off, we
would transmit the byte 0b1111_1111 (note Q7 will not affect the
displayed pattern), which corresponds to 8 pulses of the SPI CLK net
with the SPI MOSI net constantly driven HIGH.

Assuming all nets are initially driven LOW, we could:

1. Drive SPI MOSI HIGH
2. Create a rising edge on the SPI CLK net 8 times:
   a. Drive SPI CLK HIGH
   b. Drive SPI CLK LOW (to prepare for next rising edge)
3. Create a rising edge on the DISP LATCH:
   a. Drive DISP LATCH HIGH

If instead, you wanted to turn some segments on, you would need to drive
the SPI MOSI net to the appropriate state before creating each rising
edge on the SPI CLK net.

In this way, you can independently set each bit in the internal memory
of the shift register (and consequently, the corresponding output state
Q0-Q7).

TASK: Write assembly code below to configure the display on the QUTy to
display the final digit of your student number.

If the final digit of your student number is EVEN, it should be
displayed on the left-hand-side (LHS) digit of the display.

If the final digit of your student number is ODD, it should be displayed
on the right-hand-side (RHS) digit of the display.

EXAMPLE: If your student number was n12345678:

    "8" should be displayed on the LHS digit.

NOTE: Efficiency is not assessed in this exercise and therefore it is
not necessary to implement loops or subroutines.
*/

entry:
    /** CODE: Write your code for Ex E2.0 below this line. */

    ldi     r16, PIN2_bm // MOSI -> low (PC2)
    sts     PORTC_OUTCLR, r16 //clear MOSI
    ldi     r16, PIN0_bm // CLK -> low (PC0)
    sts     PORTC_OUTCLR, r16 //clear CLK
    ldi     r16, PIN1_bm  // LATCH -> low (PA1)
    sts     PORTA_OUTCLR, r16 // clear LATCH

    ldi     r16, PIN2_bm // MOSI as output
    sts     PORTC_DIRSET, r16 // enable MOSI output
    ldi     r16, PIN0_bm  // CLK as output
    sts     PORTC_DIRSET, r16 // enable CLK output
    ldi     r16, PIN1_bm  // LATCH as output
    sts     PORTA_DIRSET, r16 // enable LATCH output

    ldi     r18, 0x6B // byte to send (lights segments B & C)
    ldi     r23, 8 // send 8 bits, MSB first

.shift_loop:
    // look at MSB of r18 and set MOSI accordingly
    sbrs    r18, 7
    rjmp    .mosi_low
.mosi_high:
    ldi     r16, PIN2_bm
    sts     PORTC_OUTSET, r16 // MOSI = 1 (drive high)
    rjmp    .clk_pulse
.mosi_low:
    ldi     r16, PIN2_bm
    sts     PORTC_OUTCLR, r16 // MOSI = 0 (drive low)

.clk_pulse:
    ldi     r16, PIN0_bm // pulse CLK
    sts     PORTC_OUTSET, r16 // CLK = 1 (rising edge)
    sts     PORTC_OUTCLR, r16 // CLK = 0 (back to low)

    lsl     r18 // shift next bit into MSB
    dec     r23
    brne    .shift_loop

    ldi     r16, PIN1_bm  // toggle LATCH to update display
    sts     PORTA_OUTSET, r16 // LATCH = 1
    sts     PORTA_OUTCLR, r16 // LATCH = 0

    /** CODE: Write your code for Ex E2.0 above this line. */

// END OF EXTENSION02 EXERCISES //
// DO NOT EDIT BELOW THIS LINE  //
    break

loop:
    rjmp loop // Loop indefinitely

