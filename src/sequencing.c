#include "sequencing.h"
#include "display.h" 
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include "adc.h"
#include <stdint.h>

static uint32_t lfsr_state = 0x11993251u;
#define LFSR_MASK 0xE2025CABu

const uint16_t step_freq[4] = { 358, 301, 478, 179 };

void sequencing_init(uint32_t seed) {
    lfsr_state = (seed == 0) ? 1u : seed;
}

uint32_t sequencing_save_state(void) {
    return lfsr_state;
}

void sequencing_restore_state(uint32_t state) {
    lfsr_state = state;
}

uint8_t sequencing_next_step(void) {
    uint8_t bit = lfsr_state & 1u;
    lfsr_state >>= 1;
    if (bit) lfsr_state ^= LFSR_MASK;
    return lfsr_state & 0x03u;
}

void play_step(uint8_t step, uint16_t step_delay_ms) {
    extern volatile uint16_t elapsed_time;
    static const uint8_t left_patterns[4] = {DISP_BAR_LEFT, DISP_BAR_RIGHT, DISP_OFF, DISP_OFF};
    static const uint8_t right_patterns[4] = {DISP_OFF, DISP_OFF, DISP_BAR_LEFT, DISP_BAR_RIGHT};
    
    step &= 0x03;
    uint16_t half_delay = step_delay_ms >> 1;
    
    buzzer_on(step);
    display_set(left_patterns[step], right_patterns[step]);
    
    elapsed_time = 0;
    while (elapsed_time < half_delay) {}
    
    display_blank();
    buzzer_stop();
    elapsed_time = 0;
    while (elapsed_time < half_delay) {}
}
