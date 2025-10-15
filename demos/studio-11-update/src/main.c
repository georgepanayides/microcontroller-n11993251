#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "buzzer.h"
#include "initialisation.h"
#include "timers.h"
#include "uart.h"

static void state_machine(void);

/*** STUDIO: 11

The following program implements a simple music player. The user can
play two tones using pushbuttons S1 and S2.

Pressing S1 selects the first tone at frequency 440 Hz.
Pressing S2 selects the second tone at frequency 659 Hz.
Pressing S3 decreases the octave of both tones by 1.
Pressing S4 increases the octave of both tones by 1.

The selected tone is played back for a duration of playback delay or
until the pushbutton is released, whichever is longer.

The following commands are also supported over the serial interface:
     SELECT: '1' or '2' modifies the selected tone.
       PLAY: 'p' plays the selected tone.
       STOP: 's' stops playback immediately.
    FREQDEC: ',' decreases the octave of both tones by 1.
    FREQINC: '.' increases the octave of both tones by 1.
UPDATEDELAY: 'd' to begin receiving a payload of 4 hexadecimal digits.
             The payload is interpreted as a 16-bit unsigned integer and
             used to update the playback delay.

Receiving SELECT/FREQDEC/FREQINC while a tone is playing will cause the
playing tone to update.
*/
int main()
{
    cli();
    buttons_init();
    port_init();
    pwm_init();
    timers_init();
    uart_init();
    sei();

    state_machine();
}

// -------------------------  STATE MACHINE  -------------------------

typedef enum
{
    PAUSED,
    PLAYING
} State;

static void state_machine(void)
{
    State STATE = PAUSED;

    // Pushbutton states
    uint8_t pb_state_prev = 0xFF;
    uint8_t pb_state_curr = 0xFF;

    uint8_t pb_current = 0;

    // Pushbutton flags
    uint8_t pb_falling_edge, pb_rising_edge, pb_released = 0;

    while (1)
    {
        // Save state from previous iteration
        pb_state_prev = pb_state_curr;
        // Read current state
        pb_state_curr = pb_debounced_state;

        // Find edges
        pb_falling_edge = (pb_state_prev ^ pb_state_curr) & pb_state_prev;
        pb_rising_edge = (pb_state_prev ^ pb_state_curr) & pb_state_curr;

        // S3 and S4 may be pressed in either state
        if (pb_falling_edge & PIN6_bm)
            decrease_octave();
        else if (pb_falling_edge & PIN7_bm)
            increase_octave();

        // State machine
        switch (STATE)
        {
        case PAUSED:
            // Wait for press
            if (pb_falling_edge & (PIN4_bm | PIN5_bm))
            {
                if (pb_falling_edge & PIN4_bm)
                    pb_current = 1;
                else if (pb_falling_edge & PIN5_bm)
                    pb_current = 2;

                play_tone(pb_current - 1);

                // Update flags
                pb_released = 0;
                prepare_delay();

                // State transition
                STATE = PLAYING;
            }
            else if (uart_play == 1)
            {
                // Play whatever was previously selected
                play_selected_tone();
                prepare_delay();

                // Update flags
                pb_released = 1;
                uart_play = 0;

                // State transition
                STATE = PLAYING;
            }
            break;
        case PLAYING:
            if (uart_stop == 1)
            {
                stop_tone();
                STATE = PAUSED;
                uart_stop = 0;
            }
            else if (!pb_released)
            {
                // Wait for release
                if (pb_rising_edge & PIN4_bm && pb_current == 1)
                    pb_released = 1;
                else if (pb_rising_edge & PIN5_bm && pb_current == 2)
                    pb_released = 1;
            }
            else
            {
                // Stop if elapsed time is greater than playback time
                if (elapsed_time >= playback_delay)
                {
                    stop_tone();
                    STATE = PAUSED;
                }
            }
            break;
        default:
            STATE = PAUSED;
            stop_tone();
            break;
        }
    }
}
