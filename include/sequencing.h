#ifndef SEQUENCING_H
#define SEQUENCING_H

#include <stdint.h>

/* Initialise the 32-bit LFSR with your seed (student number in hex). */
void sequencing_init(uint32_t seed);
extern const uint16_t step_freq[4];

/* Save/restore LFSR state (for regenerating sequences without storing steps) */
uint32_t sequencing_save_state(void);
void sequencing_restore_state(uint32_t state);

/* Generate next step in [0..3]. */
uint8_t sequencing_next_step(void);

/* Generate a sequence of steps from a saved state */
void sequencing_generate_sequence(uint32_t start_state, uint8_t length, uint8_t *steps_array);

void play_step(uint8_t step, uint16_t step_delay_ms);

#endif
