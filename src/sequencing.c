#include "sequencing.h"
#include "display.h" 
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include "adc.h"
#include <stdint.h>

static uint32_t lfsr_state = 0x11993251u;
#define LFSR_MASK 0xE2025CABu

const uint16_t step_freq[4] = { 358, 301, 478, 179 }; // 3333333 / these frequencies

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

void sequencing_generate_sequence(uint32_t start_state, uint8_t length, uint8_t *steps_array) {
    sequencing_restore_state(start_state);
    for (uint8_t j = 0; j < length; j++) {
        steps_array[j] = sequencing_next_step();
    }
}

// play_step() removed - playback now handled directly in main.c state machine
