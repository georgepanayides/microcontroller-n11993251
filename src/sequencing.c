#include "sequencing.h"
#include "display.h" 
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>

// 32-bit LFSR for sequence generation
static uint32_t lfsr_state = 0x11993251u;
#define LFSR_MASK 0xE2025CABu

// Step frequencies (Hz)
const uint16_t step_freq[4] = { 338, 284, 451, 169 };

// Display patterns for each step
static const uint8_t step_is_lhs[4] = { 1, 1, 0, 0 };
static const uint8_t step_mask[4] = {
    DISP_SEG_E & DISP_SEG_F,  // S1
    DISP_SEG_B & DISP_SEG_C,  // S2  
    DISP_SEG_E & DISP_SEG_F,  // S3
    DISP_SEG_B & DISP_SEG_C   // S4
};

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
    step &= 0x03;
    
    // Start tone
    uint16_t freq = uart_apply_freq_offset(step_freq[step]);
    buzzer_start_hz(freq);
    
    // Show pattern
    if (step_is_lhs[step]) {
        display_set(step_mask[step], DISP_OFF);
    } else {
        display_set(DISP_OFF, step_mask[step]);
    }
    
    // ON for half the delay
    uint32_t t0 = millis();
    while ((millis() - t0) < (step_delay_ms / 2)) {}
    
    // OFF for remaining half
    display_blank();
    buzzer_stop();
    while ((millis() - t0) < step_delay_ms) {}
}

void play_sequence(const uint8_t *seq, uint8_t len, uint16_t step_delay_ms) {
    for (uint8_t i = 0; i < len; i++) {
        play_step(seq[i], step_delay_ms);
    }
}

/* ------------------- Non-blocking step player ------------------- */
static inline void show_step(uint8_t step) {
    /* Apply frequency offset to base tone */
    uint16_t base_hz = step_freq[step];
    uint16_t adjusted_hz = uart_apply_freq_offset(base_hz);
    /* Start tone first, then illuminate segments */
    buzzer_start_hz(adjusted_hz);

    /* Set BOTH sides of display buffer: active side shows pattern, other side is blank */
    if (step_is_lhs[step]) {
        display_set(step_mask[step], DISP_OFF);  /* LHS active, RHS blank */
    } else {
        display_set(DISP_OFF, step_mask[step]);  /* LHS blank, RHS active */
    }
    /* Rely on ISR-driven multiplex to latch shortly after */
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
