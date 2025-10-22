#include <avr/io.h>
#include "adc.h"

// /* POT is on ADC0 AIN2 */
// void adc_init_pot_8bit(void)
// {
//     /* ADC clock: 20 MHz / 32 = 625 kHz */
//     ADC0.CTRLA = 0;
//     ADC0.CTRLB = ADC_PRESC_DIV32_gc;
//     ADC0.CTRLC = (4 << ADC_TIMEBASE_gp)
//                | ADC_REFSEL_VDD_gc;
//     ADC0.CTRLE = 32;
//     // ADC0.CTRLF = ADC_FREERUN_bm;
//     ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;

//     ADC0.CTRLA = ADC_ENABLE_bm;
//     // ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;

//     // /* Warm-up: discard initial samples */
//     // for (uint8_t i = 0; i < 4; i++) {
//     //     while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) { }
//     //     (void)ADC0.RESULT0;
//     //     ADC0.INTFLAGS = ADC_RESRDY_bm;
//     // }
// }

void adc_init(void) {
    ADC0.CTRLA = ADC_ENABLE_bm;            // Enable ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;        // /2 clock prescaler 

    // Need 4 CLK_PER cycles @ 3.3 MHz for 1us, select VDD as ref
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;

    ADC0.CTRLE = 64;                               // Sample duration of 64
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;  // Free running, left adjust result
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;              // Select AIN2 (potentiomenter R1)

    // Select 12-bit resolution, single-ended
    ADC0.COMMAND = ADC_MODE_SINGLE_12BIT_gc | ADC_START_IMMEDIATE_gc;    
}


uint8_t adc_read8(void)
{
    // Clear any stale conversion flag, then wait for a fresh conversion
    // ADC0.INTFLAGS = ADC_RESRDY_bm;
    while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) { }
    uint8_t result = ADC0.RESULT0;
    ADC0.INTFLAGS = ADC_RESRDY_bm;  // Clear the flag
    return result;
}
