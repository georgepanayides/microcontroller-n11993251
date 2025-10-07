#include "sequencing.h"
#include "display.h"
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include <stdint.h>
#include "uart.h"   /* for uart_apply_freq_offset */

/* ------------------- LFSR: 32-bit, given mask ------------------- */
#define LFSR_MASK 0xE2025CABu  /* per assignment mask */

/* Replace with your student number in hex for seeding */
static uint32_t lfsr_state = 0x12345678u;  

/* ------------------- Step -> segments + tones ------------------- */
/* Segments are active-low, so use & to combine. */
#define SEG_EF  (DISP_SEG_E & DISP_SEG_F)
#define SEG_BC  (DISP_SEG_B & DISP_SEG_C)

/* Exact playback tones from Table 2 with xy=51 → 4xy = 451 Hz */
const uint16_t step_freq[4] = {
    338,  /* S1: E(high)  = 451 * 2^(-5/12) */
    284,  /* S2: C#       = 451 * 2^(-8/12) */
    451,  /* S3: A        = 4xy               */
    169   /* S4: E(low)   = 451 * 2^(-17/12)  */
};


/* Digit selection: S1/S2 = LHS, S3/S4 = RHS */
static const uint8_t step_is_lhs[4] = { 1, 1, 0, 0 };

/* Segment masks for each step */
static const uint8_t step_mask[4] = {
    SEG_EF,  /* S1 */
    SEG_BC,  /* S2 */
    SEG_EF,  /* S3 */
    SEG_BC   /* S4 */
};

/* ------------------- Sequencer core ------------------- */
void sequencing_init(uint32_t seed) {
    if (seed == 0) seed = 0x1u;  /* avoid lock-up */
    lfsr_state = seed;
}

static inline uint8_t lfsr_next_bit(void) {
    uint32_t taps = lfsr_state & LFSR_MASK;
    uint8_t parity = __builtin_parity(taps);
    lfsr_state = (lfsr_state >> 1) | ((uint32_t)parity << 31);
    return (uint8_t)(lfsr_state & 1u);
}

uint8_t sequencing_next_step(void) {
    /* 2 bits per step => 0..3 */
    uint8_t v = (lfsr_next_bit() << 1) | lfsr_next_bit();
    return v & 0x03;
}

/* ------------------- Blocking step/sequence playback ------------------- */
void play_step(uint8_t step, uint16_t step_delay_ms) {
    step &= 0x03;

    if (step_is_lhs[step]) display_write_lhs(step_mask[step]);
    else                   display_write_rhs(step_mask[step]);

    buzzer_start_hz(uart_apply_freq_offset(step_freq[step]));
    delay_ms(step_delay_ms / 2);

    display_blank();
    buzzer_stop();
    delay_ms(step_delay_ms / 2);
}

void play_sequence(const uint8_t *seq, uint8_t len, uint16_t step_delay_ms) {
    for (uint8_t i = 0; i < len; i++) {
        play_step(seq[i], step_delay_ms);
    }
}

/* ------------------- Non-blocking step player ------------------- */
static inline void show_step(uint8_t step) {
    if (step_is_lhs[step]) display_write_lhs(step_mask[step]);
    else                   display_write_rhs(step_mask[step]);
    buzzer_start_hz(uart_apply_freq_offset(step_freq[step]));
}

void sp_start(step_player_t *sp, uint8_t step, uint16_t step_delay_ms) {
    sp->step = (step & 3);
    sp->step_delay_ms = step_delay_ms;
    sp->phase = SP_ON;
    show_step(sp->step);
    sp->t_next = millis() + (step_delay_ms / 2);
}

void sp_tick(step_player_t *sp) {
    if (sp->phase == SP_IDLE) return;
    uint32_t now = millis();

    if (now - sp->t_next < 0x80000000UL) return;  /* not yet */

    if (sp->phase == SP_ON) {
        display_blank();
        buzzer_stop();
        sp->phase = SP_OFF;
        sp->t_next = now + (sp->step_delay_ms / 2);
    } else {
        sp->phase = SP_IDLE;
    }
}

uint8_t sp_done(const step_player_t *sp) {
    return (sp->phase == SP_IDLE);
}
