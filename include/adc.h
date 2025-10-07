// include/adc.h
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* Configure ADC0 for 8-bit free-running reads on the POT (AIN2), safe at 20 MHz */
void adc_init_pot_8bit(void);

/* Read current 8-bit ADC sample (0..255); clockwise ≈ larger value on QUTy */
uint8_t adc_read8(void);

/* Map 0..255 -> 250..2000 ms (linear). Clockwise => longer delay. */
static inline uint16_t playback_delay_ms_from_adc8(uint8_t x)
{
    /* 250 + x * 1750 / 255 */
    return (uint16_t)(250u + ((uint32_t)x * 1750u) / 255u);
}

#endif
