#include <avr/io.h>
#include "buzzer.h"

#define MIN_OCTAVE 0
#define MAX_OCTAVE 5

volatile uint8_t octave = 1;

void buzzer_init(void) {

    // TCA0 will control the buzzer (PB0), so we need to enable it as an output
    PORTB.OUTCLR = PIN0_bm; // buzzer off initially
    PORTB.DIRSET = PIN0_bm; // Enable PB0 as output

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;   //prescaler = 1

    // Single-slope PWM mode, WO0 enable (PB0, BUZZER)    
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // PWM initially off
    TCA0.SINGLE.PER = 1;      
    TCA0.SINGLE.CMP0 = 0;    

    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;      // Enable TCA0 
}//buzzer_init

void play_tone(uint8_t tone)
{
    // Frequency (Hz): tone0, tone2, tone2, tone3 : octave
    //                   165, 138.5,   220,  82.5 : 0
    //                   330,   277,   440,   165 : 1  (default)
    //                   660,   554,   880,   330 : 2
    //                  1320,  1108,  1760,   660 : 3
    //                  2640,  2216,  3520,  1320 : 4 
    //                  5280,  4432,  7040,  2640 : 5

    static const uint16_t per_values[4]  = {20202, 24068, 15152, 40404};

    uint16_t per = per_values[tone] >> octave;

    TCA0.SINGLE.PERBUF = per;
    TCA0.SINGLE.CMP0BUF = per >> 1;
}//play_tone

void stop_tone(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
}//stop_tone

void increase_octave(void)
{
    if (octave < MAX_OCTAVE)
        octave++;
}//increase_octave

void decrease_octave(void)
{
    if (octave > 0)
        octave--;
}//decrease_octave