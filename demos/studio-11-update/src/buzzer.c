#include "buzzer.h"

#include <stdint.h>

#include <avr/io.h>

// -----------------------------  BUZZER  -----------------------------

#define MAX_OCTAVE 3
#define MIN_OCTAVE -3

volatile uint8_t is_playing = 0;
static int8_t selected_tone = 0;
static int8_t octave = 0;

void increase_octave(void)
{
    if (octave < MAX_OCTAVE)
    {
        octave++;

        if (is_playing)
            play_tone(selected_tone);
    }
}

void decrease_octave(void)
{
    if (octave > MIN_OCTAVE)
    {
        octave--;

        if (is_playing)
            play_tone(selected_tone);
    }
}

void update_tone(uint8_t new_tone)
{
    // Update the tone if already active
    if (is_playing)
        play_tone(new_tone);
    else
        // otherwise, select a new tone for the next time a tone is played
        selected_tone = new_tone;
}

void play_selected_tone(void)
{
    play_tone(selected_tone);
}

void play_tone(uint8_t tone)
{
    // Frequencies: 440 Hz, 659 Hz
    // Periods at octave -3: 60606, 40450
    static const uint16_t periods[2] = {60606, 40450};

    uint16_t period = periods[tone] >> (octave + 3);
    TCA0.SINGLE.PERBUF = period;
    TCA0.SINGLE.CMP0BUF = period >> 1;

    selected_tone = tone;
    is_playing = 1;
}

void stop_tone(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
    is_playing = 0;
}
