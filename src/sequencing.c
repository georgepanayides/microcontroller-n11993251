#include "sequencing.h"
#include "display.h"
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include <stdint.h>
#include <stdio.h>
#include "uart.h"   /* for uart_apply_freq_offset */
/* For SREG and cli/sei */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Internal 32-bit LFSR RNG per assignment spec Section B */
static uint32_t lfsr_state = 0x11993251u; /* seed with student number */

/* Assignment-specified mask for the 32-bit LFSR */
#define LFSR_MASK 0xE2025CABu

/* Segments are active-low; combine with bitwise AND */
#define SEG_EF  (DISP_SEG_E & DISP_SEG_F)
#define SEG_BC  (DISP_SEG_B & DISP_SEG_C)

/* Exact playback tones from Table 2 with xy=51 (4xy -> 451 Hz)
   E(high) = 451 * 2^(-5/12)  ~ 338 Hz
   C#      = 451 * 2^(-8/12)  ~ 284 Hz
   A       = 451              = 451 Hz
   E(low)  = 451 * 2^(-17/12) ~ 169 Hz
*/
const uint16_t step_freq[4] = {
    338,  /* S1: E(high) */
    284,  /* S2: C#      */
    451,  /* S3: A       */
    169   /* S4: E(low)  */
};


/* Digit selection: S1/S2 = LHS, S3/S4 = RHS */
static const uint8_t step_is_lhs[4] = { 1, 1, 0, 0 };

/* Segment masks for each step (active-low) */
static const uint8_t step_mask[4] = {
    SEG_EF,  /* S1 */
    SEG_BC,  /* S2 */
    SEG_EF,  /* S3 */
    SEG_BC   /* S4 */
};

/* ------------------- Sequencer core ------------------- */
void sequencing_init(uint32_t seed) {
    if (seed == 0) seed = 1u;
    lfsr_state = seed;
}

/* Save current LFSR state (for regenerating sequence without array storage) */
uint32_t sequencing_save_state(void) {
    return lfsr_state;
}

/* Restore LFSR state (for regenerating sequence without array storage) */
void sequencing_restore_state(uint32_t state) {
    lfsr_state = state;
}

/* Assignment-specified LFSR algorithm from Section B */
uint8_t sequencing_next_step(void) {
    uint8_t bit = (uint8_t)(lfsr_state & 1u);  /* BIT ← lsbit(STATE_LFSR) */
    lfsr_state >>= 1;                          /* STATE_LFSR ← STATE_LFSR >> 1 */
    if (bit == 1u) {
        lfsr_state ^= LFSR_MASK;               /* if (BIT = 1) STATE_LFSR ← STATE_LFSR xor MASK */
    }
    return (uint8_t)(lfsr_state & 0x03u);      /* STEP ← STATE_LFSR and 0b11 */
}

/* ------------------- Blocking step/sequence playback ------------------- */
void play_step(uint8_t step, uint16_t step_delay_ms) {
    step &= 0x03;

    // DEBUG: Uncomment for timing analysis
    // uart_puts("[STEP] BEGIN step=");
    // uart_put_u16_dec((uint16_t)(step+1));
    // uart_puts(" at t=");
    // uart_put_u32_dec(millis());
    // uart_puts("ms delay=");
    // uart_put_u16_dec(step_delay_ms);
    // uart_puts("ms\r\n");

    /* Apply frequency offset to base tone */
    uint16_t base_hz = step_freq[step];
    uint16_t active_hz = uart_apply_freq_offset(base_hz);

    /* Start tone first, then illuminate segments */
    buzzer_start_hz(active_hz);

    /* Set BOTH sides of display buffer: active side shows pattern, other side is blank */
    if (step_is_lhs[step]) {
        display_set(step_mask[step], DISP_OFF);  /* LHS active, RHS blank */
    } else {
        display_set(DISP_OFF, step_mask[step]);  /* LHS blank, RHS active */
    }
    /* Rely on ISR-driven multiplex to latch shortly after */

    uint32_t t0 = millis();
    uint32_t on_time = step_delay_ms / 2;
    
    // uart_puts("[DBG] ON start t0=");
    // uart_put_u32_dec(t0);
    // uart_puts(" target=");
    // uart_put_u32_dec(on_time);
    // uart_puts("ms\r\n");
    
    /* Wait for ON phase */
    while ((uint32_t)(millis() - t0) < on_time) {
        /* While ON, accept INC/DEC and retune immediately per spec */
        uint16_t before = active_hz;
        uart_poll_commands(0, base_hz, &active_hz);
        if (active_hz != before) {
            buzzer_start_hz(active_hz); /* retune live */
        }
        /* ISR continues display multiplexing */
    }
    
    /* Turn off display and buzzer */
    // uart_puts("[DBG] display_blank() at t=");
    // uart_put_u32_dec(millis() - t0);
    // uart_puts(" (expected=");
    // uart_put_u32_dec(on_time);
    // uart_puts(")\r\n");
    display_blank();
    buzzer_stop();
    
    /* Wait for OFF phase */
    while ((uint32_t)(millis() - t0) < step_delay_ms) {
        /* Spin */
    }
    
    // uart_puts("[DBG] play_step complete total=");
    // uart_put_u32_dec(millis() - t0);
    // uart_puts("ms\r\n");
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
