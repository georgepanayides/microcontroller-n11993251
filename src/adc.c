#include <avr/io.h>
#include "adc.h"

void adc_init(void) {
    ADC0.CTRLA = ADC_ENABLE_bm;            // Enable ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;        // /2 clock prescaler 

    // Need 4 CLK_PER cycles @ 3.3 MHz for 1us, select VDD as ref
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;

    ADC0.CTRLE = 64;                               // Sample duration of 64
    ADC0.CTRLF = ADC_FREERUN_bm;                   // Free running (right-adjusted for >> 8 shift)
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;              // Select AIN2 (potentiomenter R1)

    // Select 8-bit resolution, single-ended (matches dice-game for >> 8 calculation)
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;    
}