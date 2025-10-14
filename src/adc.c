// src/adc.c
#include <avr/io.h>
#include "adc.h"

/* POT is on ADC0 AIN2 on QUTy */
void adc_init_pot_8bit(void)
{
    /* Safe ADC clock: 20 MHz / 32 = 625 kHz (within datasheet range) */
    ADC0.CTRLA = 0;                         /* disable while configuring */
    ADC0.CTRLB = ADC_PRESC_DIV32_gc;        /* prescaler */
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp)     /* timebase ~4 cycles */
               | ADC_REFSEL_VDD_gc;         /* Vref = VDD */
    ADC0.CTRLE = 32;                        /* sample duration */
    ADC0.CTRLF = ADC_FREERUN_bm;            /* free-run mode */
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;       /* POT input */

    ADC0.CTRLA = ADC_ENABLE_bm;             /* enable ADC */
    /* 8-bit single-ended, start immediately (free-run keeps it going) */
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;

    /* Warm-up: discard a few initial samples so first read is valid */
    for (uint8_t i = 0; i < 4; i++) {
        while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) { /* wait */ }
        (void)ADC0.RESULT0;
        ADC0.INTFLAGS = ADC_RESRDY_bm;
    }
}

uint8_t adc_read8(void)
{
    return ADC0.RESULT0;                    /* 0..255 */
}
