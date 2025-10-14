// buzzer.c
#include <avr/io.h>
#include "buzzer.h"

#ifndef F_CPU
#  define F_CPU 3333333UL  /* Default QUTy platform clock */
#endif

/* Calculate PER and CMP for 50% duty cycle at given frequency
   Based on buzzer-demo/extension05.c implementation */
static inline void tca_calc_50pct(uint16_t hz, uint16_t *per_out, uint16_t *cmp_out)
{
    if (hz == 0) {
        *per_out = 0;
        *cmp_out = 0;
        return;
    }
    
    /* PER = (F_CPU / hz) - 1, with rounding */
    uint32_t per = (F_CPU + (hz / 2u)) / (uint32_t)hz;
    if (per == 0) per = 1;
    per -= 1;
    
    if (per > 0xFFFFUL) per = 0xFFFFUL;
    uint16_t per16 = (uint16_t)per;
    
    /* 50% duty cycle */
    uint16_t cmp16 = (uint16_t)((per16 + 1u) / 2u);
    
    *per_out = per16;
    *cmp_out = cmp16;
}

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

        // Prime period and duty to safe, immediate values to avoid any first-update latency.
        // Setting PER to 1 ensures next updates don't wait for a long overflow, and CMP0=0 keeps output silent.
        TCA0.SINGLE.PER = 1;
        TCA0.SINGLE.CMP0 = 0;

        // Use DIV1 (no prescaler) to match demo implementation
        // TCA clock = F_CPU (3.33 MHz)
        TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
    }
}

void buzzer_init(void) {
    tca0_enable_if_needed();
    // Maintain silence explicitly
    TCA0.SINGLE.CMP0 = 0;
}

void buzzer_start_hz(uint16_t hz) {
    tca0_enable_if_needed();
    if (hz == 0) { 
        TCA0.SINGLE.CMP0 = 0; 
        return; 
    }

    uint16_t per, cmp;
    tca_calc_50pct(hz, &per, &cmp);
    
    /* Program period and duty first, reset counter, then enable output */
    TCA0.SINGLE.PER = per;
    TCA0.SINGLE.CNT = 0;
    TCA0.SINGLE.CMP0 = cmp;
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
}

void buzzer_stop(void) {
    /* Drive silent and disable output so no residual toggling is detected */
    TCA0.SINGLE.CMP0 = 0;
    TCA0.SINGLE.CTRLB &= (uint8_t)~TCA_SINGLE_CMP0EN_bm;
    /* Force pin low so harness sees a stable 0 Hz (no toggling) */
    PORTB.OUTCLR = PIN0_bm;
}

/* Legacy function using buffered registers (for glitch-free transitions) */
void buzzer_on(uint8_t tone_index)
{
    extern const uint16_t step_freq[4];  /* defined in sequencing.c */
    tca0_enable_if_needed();
    
    uint8_t idx = tone_index & 0x03;
    uint16_t hz = step_freq[idx];
    
    uint16_t per, cmp;
    tca_calc_50pct(hz, &per, &cmp);
    
    /* Use buffered registers for smooth transitions */
    TCA0.SINGLE.CMP0BUF = cmp;
    TCA0.SINGLE.PERBUF = per;
}

void buzzer_off(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
}

