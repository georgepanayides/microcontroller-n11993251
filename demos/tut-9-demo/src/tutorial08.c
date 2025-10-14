#include <avr/io.h>
#include "timer.h"

// states
typedef enum { OFF, CONFIRM_ON, ON, CONFIRM_OFF } state_t;

int main(void)
{
    buttons_init();
    timer_init();

    // dp led on pb5
    PORTB.DIRSET = PIN5_bm;     // out
    PORTB.OUTSET = PIN5_bm;     // off (active-low on many boards)

    state_t state = OFF;        // start here

    static uint8_t prev_state = 0xFF;  // all released

    while (1)
    {
        // take one coherent snapshot (avoid torn reads vs isr)
        uint8_t curr_state = pb_debounced_state;

        // falling edge = 1 -> 0 (released -> pressed)
        uint8_t pb_falling_edge = prev_state & (prev_state ^ curr_state);

        // remember for next loop
        prev_state = curr_state;

        // fsm
        switch (state)
        {
            case OFF:
                if (pb_falling_edge & PIN4_bm) // Only S1 starts confirmation
                {
                    state = CONFIRM_ON;
                }
                break;

            case CONFIRM_ON:
                if (pb_falling_edge & PIN5_bm) // S2 confirms ON
                {
                    state = ON;
                }
                else if (pb_falling_edge & (PIN4_bm | PIN6_bm | PIN7_bm)) // S1, S3, or S4 pressed
                {
                    state = OFF;
                }
                break;

            case ON:
                if (pb_falling_edge & PIN6_bm) // S3 starts OFF confirmation
                {
                    state = CONFIRM_OFF;
                }
                break;

            case CONFIRM_OFF:
                if (pb_falling_edge & PIN7_bm) // S4 confirms OFF
                {
                    state = OFF;
                }
                else if (pb_falling_edge & (PIN4_bm | PIN5_bm | PIN6_bm)) // S1, S2, or S3 pressed
                {
                    state = ON;
                }
                break;

            default:
                state = OFF;
                break;
        }
    }
}
