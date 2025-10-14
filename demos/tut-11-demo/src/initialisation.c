// initialisation.c

#include "initialisation.h"

#include <avr/io.h>

void adc_init(void)
{
    ADC0.CTRLA = ADC_ENABLE_bm;                              // Enable ADC0
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;                          // Set prescaler to 2
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc; // Set reference voltage to VDD
    ADC0.CTRLE = 64;                                         // Sample duration
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;                        // Select ADC channel AIN2 (PA2)
}
