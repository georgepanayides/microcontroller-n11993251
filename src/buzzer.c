#include <avr/io.h>
#include "buzzer.h"

#ifndef F_CPU
#  define F_CPU 3333333UL
#endif

/* Calculate PER and CMP for 50% duty cycle */
static inline void tca_calc_50pct(uint16_t hz, uint16_t *per_out, uint16_t *cmp_out)
{
    if (hz == 0) {
        *per_out = 0;
        *cmp_out = 0;
        return;
    }
    
    /* PER = (F_CPU / hz) - 1 */
    uint32_t per = (F_CPU + (hz / 2u)) / (uint32_t)hz;
    if (per == 0) per = 1;
    per -= 1;
    
    if (per > 0xFFFFUL) per = 0xFFFFUL;
    uint16_t per16 = (uint16_t)per;
    
    uint16_t cmp16 = (uint16_t)((per16 + 1u) / 2u);
    
    *per_out = per16;
    *cmp_out = cmp16;
}

static inline void tca0_enable_if_needed(void) {
    if (!(TCA0.SINGLE.CTRLA & TCA_SINGLE_ENABLE_bm)) {
        PORTMUX.TCAROUTEA = PORTMUX_TCA00_DEFAULT_gc;
        PORTB.DIRSET = PIN0_bm;
        TCA0.SINGLE.CTRLA = 0;
        TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
        TCA0.SINGLE.PER = 1;
        TCA0.SINGLE.CMP0 = 0;
        TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
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
    
    // Use buffered registers only (like studio demo)
    TCA0.SINGLE.PERBUF = per;
    TCA0.SINGLE.CMP0BUF = cmp;
}

void buzzer_stop(void) {
    TCA0.SINGLE.CMP0BUF = 0;
    PORTB.OUTCLR = PIN0_bm;
}

