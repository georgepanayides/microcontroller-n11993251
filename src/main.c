// src/main.c
#include <avr/io.h>
#include <avr/interrupt.h>
#include "initialisation.h"
#include "display.h"
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include "sequencing.h"
#include "buttons.h"
#include "adc.h"
#include "uart.h"
#include "highscore.h"

static void state_machine(void);

#define STEP_DELAY_MS     1200
#define ROUND_PAUSE_MS    1200
#define INPUT_GUARD_MS       5
#define ECHO_RELEASE_THRESHOLD_MS 30

#define SUCCESS_TONE_HZ   1200
#define FAIL_TONE_HZ       400

static uint8_t digit_mask(uint8_t d)
{
    switch (d % 10) {
    case 0: return DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_E & DISP_SEG_F;
    case 1: return DISP_SEG_B & DISP_SEG_C;
    case 2: return DISP_SEG_A & DISP_SEG_B & DISP_SEG_G & DISP_SEG_E & DISP_SEG_D;
    case 3: return DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_G;
    case 4: return DISP_SEG_F & DISP_SEG_G & DISP_SEG_B & DISP_SEG_C;
    case 5: return DISP_SEG_A & DISP_SEG_F & DISP_SEG_G & DISP_SEG_C & DISP_SEG_D;
    case 6: return DISP_SEG_A & DISP_SEG_F & DISP_SEG_E & DISP_SEG_D & DISP_SEG_C & DISP_SEG_G;
    case 7: return DISP_SEG_A & DISP_SEG_B & DISP_SEG_C;
    case 8: return DISP_ON;
    default:return DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_F & DISP_SEG_G;
    }
}

static void display_score_decimal_for(uint16_t score, uint16_t ms)
{
    uint8_t show = (uint8_t)(score % 100);
    uint8_t tens = show / 10, ones = show % 10;
    uint8_t left_mask  = (tens == 0 && score < 100) ? DISP_OFF : digit_mask(tens);
    uint8_t right_mask = digit_mask(ones);
    display_set(left_mask, right_mask);
    uint32_t t0 = millis();
    while ((uint32_t)(millis() - t0) < ms) {}
    display_blank();
}

int main(void)
{
    cli();
    gpio_init();
    spi_init_for_display();
    tcb0_init_1ms();
    uart_init_9600_8n1();
    buttons_init();
    display_init();
    buzzer_init();
    adc_init_pot_8bit();
    buzzer_stop();
    sequencing_init(0x11993251u);
    sei();
    state_machine();
    while (1) {}
}

static void echo_button(uint8_t b, uint16_t step_delay_ms,
                        uint16_t *last_base_hz, uint16_t *active_hz, uint8_t sustain)
{
    extern const uint16_t step_freq[4];
    *last_base_hz = step_freq[b];
    *active_hz    = uart_apply_freq_offset(*last_base_hz);

    /* Use the round's playback delay so echo ON >= 50% of that delay */
    uint16_t min_on = (uint16_t)(step_delay_ms / 2);
    uint32_t t0;
    /* Demo parity: start the tone FIRST, then set the segments. */
    buzzer_start_hz(*active_hz);
    if (b < 2) {
        display_set((b == 0) ? (DISP_SEG_E & DISP_SEG_F)
                             : (DISP_SEG_B & DISP_SEG_C),
                    DISP_OFF);
    } else {
        display_set(DISP_OFF,
                    (b == 2) ? (DISP_SEG_E & DISP_SEG_F)
                             : (DISP_SEG_B & DISP_SEG_C));
    }
    t0 = millis();
    while ((uint32_t)(millis() - t0) < min_on) {
        uart_poll_commands(0, *last_base_hz, active_hz);
    }
    /* Sustain if still held after the minimum ON time (only when sustain != 0) */
    if (sustain) {
        while (buttons_is_down(b)) {
            uart_poll_commands(0, *last_base_hz, active_hz);
        }
    }
    buzzer_stop();
    *last_base_hz = 0; *active_hz = 0;
    display_blank();
}

static void state_machine(void)
{
    typedef enum { GS_WAIT_START, GS_PLAYBACK, GS_INPUT, GS_PAUSE, GS_FAIL } game_state_t;
    static uint32_t round_start_state = 0;
    static uint8_t len = 0;
    static uint8_t i = 0;
    game_state_t gs = GS_PLAYBACK;

    static uint16_t current_step_delay_ms = STEP_DELAY_MS;
    uint32_t guard_until = 0;
    uint32_t pause_until = 0;

    uint16_t last_base_hz = 0;
    uint16_t active_hz    = 0;

    while (1) {
        uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);

        if (gs == GS_WAIT_START) {
            if (uart_take_reset_and_clear()) { sequencing_init(0x11993251u); len = 0; }
            if (uart_seed_pending()) { sequencing_init(uart_take_seed()); len = 0; }
        }

        switch (gs) {
        case GS_WAIT_START:
            display_blank();
            buzzer_stop();
            if (buttons_pop() >= 0) { len = 0; gs = GS_PLAYBACK; }
            break;

        case GS_PLAYBACK: {
            if (len == 0 && uart_seed_pending()) sequencing_init(uart_take_seed());
            /* CRITICAL: Save the LFSR state only once at game start (len==0).
               Each round replays the first (len+1) steps from this saved state. */
            if (len == 0) {
                round_start_state = sequencing_save_state();
            }
            len++;
            /* Do not clear: preserve presses during round pause and playback.
               We'll use a short guard at input start to suppress accidental carry-over. */
            /* Sample pot ONCE per round (spec) and use one delay for every step */
            {
                uint8_t pot = adc_read8();
                current_step_delay_ms = playback_delay_ms_from_adc8(pot);
            }
            sequencing_restore_state(round_start_state);
            for (uint8_t j = 0; j < len; j++) {
                uint8_t step = sequencing_next_step();
                play_step(step, current_step_delay_ms);
            }
            /* Don't clear buttons after playback - keep any presses during playback */
            i = 0;
            guard_until = millis() + INPUT_GUARD_MS;
            gs = GS_INPUT;
            break; }

        case GS_INPUT: {
            if ((int32_t)(millis() - guard_until) < 0) break;
            int8_t b = buttons_pop();
            if (b < 0) break;
            /* Compute expected step i without disturbing current global LFSR state */
            uint32_t keep = sequencing_save_state();
            sequencing_restore_state(round_start_state);
            uint8_t expected = 0;
            for (uint8_t j = 0; j <= i; j++) expected = sequencing_next_step();
            sequencing_restore_state(keep);
            echo_button((uint8_t)b, current_step_delay_ms, &last_base_hz, &active_hz, 1);
            if ((uint8_t)b == expected) {
                i++;
                if (i == len) {
                    uart_game_success(len);
                    /* Recompute playback delay for SUCCESS pattern */
                    uint8_t pot = adc_read8();
                    current_step_delay_ms = playback_delay_ms_from_adc8(pot);
                    last_base_hz = SUCCESS_TONE_HZ;
                    active_hz    = uart_apply_freq_offset(last_base_hz);
                    buzzer_start_hz(active_hz);
                    display_set(DISP_ON, DISP_ON);
                    uint32_t s0 = millis();
                    while ((uint32_t)(millis() - s0) < current_step_delay_ms) {
                        uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);
                    }
                    buzzer_stop();
                    last_base_hz = 0; active_hz = 0;
                    display_blank();
                    /* Non-blocking pause: allow short, non-sustained echo during the pause */
                    pause_until = millis() + ROUND_PAUSE_MS;
                    gs = GS_PAUSE;
                }
            } else {
                gs = GS_FAIL;
            }
            break; }

        case GS_PAUSE: {
            if ((int32_t)(millis() - pause_until) >= 0) {
                gs = GS_PLAYBACK;
                break;
            }
            int8_t b = buttons_pop();
            if (b >= 0) {
                /* Echo with sustain disabled so it completes within half-delay */
                echo_button((uint8_t)b, current_step_delay_ms, &last_base_hz, &active_hz, 0);
            }
            break; }

        case GS_FAIL: {
            buzzer_start_hz(uart_apply_freq_offset(FAIL_TONE_HZ));
            uint32_t f0 = millis();
            while ((uint32_t)(millis() - f0) < current_step_delay_ms) {
                uint16_t live = uart_apply_freq_offset(FAIL_TONE_HZ);
                uart_poll_commands((uint8_t)gs, FAIL_TONE_HZ, &live);
            }
            buzzer_stop();
            display_blank();
            display_score_decimal_for(len, current_step_delay_ms);
            delay_ms(current_step_delay_ms);
            display_blank();
            if (highscore_qualifies(len)) highscore_prompt_and_store(len);
            buttons_clear_all();
            gs = GS_WAIT_START;
            break; }
        }
    }
}
