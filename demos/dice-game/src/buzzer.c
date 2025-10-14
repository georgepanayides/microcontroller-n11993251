#include <avr/io.h>
#include "buzzer.h"

#define MIN_OCTAVE 0
#define MAX_OCTAVE 5

volatile uint8_t octave = 2;

void buzzer_init(void) {

    // TCA0 will control the buzzer (PB0), so we need to enable it as an output
    PORTB.OUTCLR = PIN0_bm; // buzzer off initially
    PORTB.DIRSET = PIN0_bm; // Enable PB0 as output

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;

    // Single-slope PWM mode, WO2 enable (PB5, LED DISP DP)    
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // Enable overflow interrupt (interrupt at TOP)
    // TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

    // PWM initially off
    TCA0.SINGLE.PER = 1;      
    TCA0.SINGLE.CMP0 = 0;    

    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;      // Enable TCA0 
}//buzzer_init


void play_tone(uint8_t tone)
{
    // Frequencies (Hz):       octave
    //   2640,  2216,  3520    5   /32
    //   1320,  1108,  1760    4   /16
    //    660,   554,   880    3   /8
    //    330,   277,   440    2   /4   default 
    //    165,   138,   220    1   /2
    //     83,    69,   110    0   /1

    // PER values for lowest frequency
    static const uint16_t per_values[3] = {40404, 48135, 30303};

    uint16_t per_val = (per_values[tone] >> octave);

    TCA0.SINGLE.PERBUF = per_val;
    TCA0.SINGLE.CMP0BUF = per_val >> 1;
}//play_tone

void stop_tone(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
}//stop_tone

void increase_octave() 
{
   octave = (octave == MAX_OCTAVE)? MAX_OCTAVE : octave+1;
}//increase_octave

void decrease_octave() 
{
   octave = (octave == MIN_OCTAVE)? MIN_OCTAVE : octave-1;
}//decrease_octave