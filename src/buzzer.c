#include <avr/io.h>
#include "buzzer.h"

// Octave shifting for Section D
static int8_t octave = 0;
#define MAX_OCTAVE 3
#define MIN_OCTAVE -3

void buzzer_init(void) {

    // TCA0 will control the buzzer (PB0), so we need to enable it as an output
    PORTB.OUTCLR = PIN0_bm; // buzzer off initially
    PORTB.DIRSET = PIN0_bm; // Enable PB0 as output

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV2_gc;   //prescaler = 2 (gives 1.666 MHz)

    // Single-slope PWM mode, WO0 enable (PB0, BUZZER)    
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // PWM initially off
    TCA0.SINGLE.PER = 1;      
    TCA0.SINGLE.CMP0 = 0;    

    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;      // Enable TCA0 
}//buzzer_init


void play_tone(uint8_t tone)
{
    // Base PER values (using DIV2 prescaler -> 1,666,666 Hz):
    // corresponding to tones: 330, 277, 440, 165 Hz
    static const uint32_t base_per_values32[4] = {5050UL, 6012UL, 3788UL, 10101UL};

    // Use 32-bit intermediate to avoid overflow/truncation when shifting
    uint32_t per32 = base_per_values32[tone];

    // Apply octave shift using 32-bit arithmetic
    if (octave > 0) {
        per32 >>= octave;  // Higher frequency => smaller PER
    } else if (octave < 0) {
        per32 <<= (uint8_t)(-octave);  // Lower frequency => larger PER
    }

    // Clamp to hardware / audible limits. PER must fit in 16-bit TCA register.
    // At 1.666 MHz: 20 kHz -> PER ~= 83, 20 Hz -> PER ~= 83333 (too large for 16-bit)
    if (per32 < 83UL) per32 = 83UL;
    if (per32 > 0xFFFFUL) per32 = 0xFFFFUL;

    uint16_t per16 = (uint16_t)per32;
    TCA0.SINGLE.PERBUF = per16;
    TCA0.SINGLE.CMP0BUF = per16 >> 1;
}//play_tone

void stop_tone(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
}//stop_tone

// Wrapper functions for Simon Says
void buzzer_on(uint8_t step) {
    play_tone(step);
}

void buzzer_stop(void) {
    stop_tone();
}

void buzzer_start_hz(uint16_t hz) {
    if (hz == 0) {
        TCA0.SINGLE.CMP0BUF = 0;
        return;
    }
    // Using DIV2 prescaler, effective clock = 1.666 MHz
    uint32_t per = (1666666UL + (hz >> 1)) / hz;
    if (per == 0) per = 1;
    per -= 1;
    if (per > 0xFFFF) per = 0xFFFF;
    
    TCA0.SINGLE.PERBUF = (uint16_t)per;
    TCA0.SINGLE.CMP0BUF = (uint16_t)((per + 1) >> 1);
}

void increase_octave(void) {
    if (octave < MAX_OCTAVE) octave++;
}

void decrease_octave(void) {
    if (octave > MIN_OCTAVE) octave--;
}

