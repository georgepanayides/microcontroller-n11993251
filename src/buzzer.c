#include <avr/io.h>
#include "buzzer.h"

#ifndef F_CPU
#  define F_CPU 3333333UL
#endif

#define MAX_OCTAVE 3
#define MIN_OCTAVE -3
static int8_t octave = 0;

static const uint16_t base_freq[4] = { 358, 301, 478, 179 };
// 1666666 / 45 = 37037
// 3333333 isnt going to work because it cant get stored in an 8 bit const 
// should just do the calc then call this in the main 
static inline void tca_calc_50pct(uint16_t hz, uint16_t *per_out, uint16_t *cmp_out)
{
    if (hz == 0) {
        *per_out = 0;
        *cmp_out = 0;
        return;
    }
    
    uint32_t per = (F_CPU + (hz >> 1)) / hz;
    if (per == 0) per = 1;
    per -= 1;
    if (per > 0xFFFF) per = 0xFFFF;
    
    *per_out = (uint16_t)per;
    *cmp_out = (uint16_t)((per + 1) >> 1);
}

static inline void tca0_enable_if_needed(void) {
    if (!(TCA0.SINGLE.CTRLA & TCA_SINGLE_ENABLE_bm)) {
        PORTMUX.TCAROUTEA = PORTMUX_TCA00_DEFAULT_gc;
        PORTB.DIRSET = PIN0_bm;
        TCA0.SINGLE.CTRLA = 0;
        TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
        TCA0.SINGLE.PER = 1;
        TCA0.SINGLE.CMP0 = 0;
        TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm; // should be using DIV2 
    }
}

void buzzer_init(void) {
    tca0_enable_if_needed();
    TCA0.SINGLE.CMP0 = 0;
}

void buzzer_start_hz(uint16_t hz) {
    tca0_enable_if_needed();
    if (hz == 0) { 
        TCA0.SINGLE.CMP0BUF = 0; 
        return; 
    }

    uint16_t per, cmp;
    tca_calc_50pct(hz, &per, &cmp);
    TCA0.SINGLE.PERBUF = per;
    TCA0.SINGLE.CMP0BUF = cmp;
}

void buzzer_stop(void) {
    TCA0.SINGLE.CMP0BUF = 0;
    PORTB.OUTCLR = PIN0_bm;
}

void increase_octave(void) {
    if (octave < MAX_OCTAVE) octave++;
}

void decrease_octave(void) {
    if (octave > MIN_OCTAVE) octave--;
}

void buzzer_on(uint8_t tone_index) {
    uint16_t freq = base_freq[tone_index & 0x03];
    
    if (octave > 0) {
        freq = freq << octave;
    } else if (octave < 0) {
        freq = freq >> (-octave);
    }
    
    buzzer_start_hz(freq);
}

void buzzer_off(void) {
    buzzer_stop();
}

