#include "sequencing.h"
#include "display.h" 
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
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
    step &= 0x03;
    
    uint16_t freq = step_freq[step];
    buzzer_start_hz(freq);
    
    switch (step) {
        case 0: display_set(DISP_BAR_LEFT, DISP_OFF); break;
        case 1: display_set(DISP_BAR_RIGHT, DISP_OFF); break;
        case 2: display_set(DISP_OFF, DISP_BAR_LEFT); break;
        case 3: display_set(DISP_OFF, DISP_BAR_RIGHT); break;
    }

    elapsed_time = 0;
    while (elapsed_time < (step_delay_ms / 2)) {}
    
    display_blank();
    buzzer_stop();
    elapsed_time = 0;
    while (elapsed_time < (step_delay_ms / 2)) {}
}
