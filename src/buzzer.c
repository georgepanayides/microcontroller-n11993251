// buzzer.c
#include <avr/io.h>
#include "buzzer.h"

#ifndef F_CPU
#  define F_CPU 20000000UL
#endif

static inline void tca0_enable_if_needed(void) {
    if (!(TCA0.SINGLE.CTRLA & TCA_SINGLE_ENABLE_bm)) {
        // Route WO0 to PB0 (default route, ensure selected)
        PORTMUX.TCAROUTEA = PORTMUX_TCA00_DEFAULT_gc;

        // PB0 as output
        PORTB.DIRSET = PIN0_bm;

        // Disable before configuring
        TCA0.SINGLE.CTRLA = 0;

        // Single-slope PWM, enable CMP0 (WO0)
        TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

        // Prescaler /8 → 2.5 MHz TCA clock, then enable
        TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc | TCA_SINGLE_ENABLE_bm;
    }
}

void buzzer_init(void) {
    tca0_enable_if_needed();
    TCA0.SINGLE.CMP0 = 0;  // silent
}

void buzzer_start_hz(uint16_t hz) {
    tca0_enable_if_needed();
    if (hz == 0) { 
        TCA0.SINGLE.CMP0 = 0; 
        return; 
    }

    // PER = (F_CPU / (prescaler * hz)) - 1
    uint32_t per_calc = (F_CPU / 8UL) / (uint32_t)hz;
    if (per_calc > 0) per_calc -= 1;
    if (per_calc > 0xFFFFUL) per_calc = 0xFFFFUL;

    uint16_t per = (uint16_t)per_calc;
    TCA0.SINGLE.PER = per;

    // 50% duty
    uint16_t half = (uint16_t)((per + 1U) >> 1);
    if (half > per) half = per;
    TCA0.SINGLE.CMP0 = half;
}

void buzzer_stop(void) {
    TCA0.SINGLE.CMP0 = 0;
}
