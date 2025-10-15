#include "timers.h"

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

// -----------------------------  TIMING  -----------------------------
volatile uint16_t elapsed_time = 0;
volatile uint16_t playback_delay = 2000;
volatile uint16_t new_playback_delay = 2000;

void prepare_delay(void)
{
    elapsed_time = 0;
    playback_delay = new_playback_delay;
}

ISR(TCB0_INT_vect)
{
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

// ----------------------  PUSH BUTTON HANDLING  ----------------------
volatile uint8_t pb_debounced_state = 0xFF;

ISR(TCB1_INT_vect)
{
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    uint8_t pb_edge = pb_debounced_state ^ PORTA.IN;

    // Vertical counter
    count1 = (count1 ^ count0) & pb_edge;
    count0 = ~count0 & pb_edge;

    // Update upon three consistent samples
    pb_debounced_state ^= (count1 & count0);

    TCB1.INTFLAGS = TCB_CAPT_bm;
}
