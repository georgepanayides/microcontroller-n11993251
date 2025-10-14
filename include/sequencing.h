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

void play_step(uint8_t step, uint16_t step_delay_ms);
void play_sequence(const uint8_t *seq, uint8_t len, uint16_t step_delay_ms);

/* Non-blocking step player (call from main loop at ~1kHz or faster) */
typedef enum { SP_IDLE, SP_ON, SP_OFF } step_player_phase_t;

typedef struct {
    step_player_phase_t phase;
    uint32_t t_next;          /* next boundary in ms (uses millis()) */
    uint16_t step_delay_ms;   /* full step duration */
    uint8_t  step;            /* 0..3 */
} step_player_t;

void sp_start(step_player_t *sp, uint8_t step, uint16_t step_delay_ms);
void sp_tick(step_player_t *sp);   /* drive display/buzzer via phases */
uint8_t sp_done(const step_player_t *sp);

#endif
