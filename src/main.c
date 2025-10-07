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
#include "uart.h"   /* <-- UART driver & game helpers */
#include "highscore.h"

static void state_machine(void);

/* ---------- Timing & tones ---------- */
#define STEP_DELAY_MS     1200              /* default; replaced each round by POT */
#define ROUND_PAUSE_MS    1200
#define INPUT_GUARD_MS      20

/* Pattern tones (OK to keep as placeholders) */
#define SUCCESS_TONE_HZ   1200
#define FAIL_TONE_HZ       400
/* No explicit win state per spec; game continues until FAIL */

/* ---------- Optional: decimal score to 7-seg (active-low masks) ---------- */
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
    default:return DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_F & DISP_SEG_G; /* 9 */
    }
}
static void display_score_decimal_for(uint16_t score, uint16_t ms)
{
    uint8_t show = (uint8_t)(score % 100);  /* two least-significant digits if >99 */
    uint8_t tens = show / 10, ones = show % 10;
    uint8_t left_mask  = (tens == 0 && score < 100) ? DISP_OFF : digit_mask(tens);
    uint8_t right_mask = digit_mask(ones);
    uint32_t t0 = millis();
    while ((uint32_t)(millis() - t0) < ms) {
        display_write_lhs(left_mask);  delay_ms(1);
        display_write_rhs(right_mask); delay_ms(1);
    }
    display_blank();
}

/* ---------- Game ---------- */
typedef enum { GS_WAIT_START, GS_PLAYBACK, GS_INPUT, GS_FAIL } game_state_t;

int main(void)
{
    cli();

    clock_init_20mhz();       /* 20 MHz, no prescale */
    gpio_init();              /* display latch pin etc. */
    spi_init_for_display();   /* SPI0 host for 74HC595 */
    tcb0_init_1ms();          /* 1 ms system tick (millis/delay_ms) */
    uart_init_9600_8n1();     /* 9600-8N1 on PB2/PB3 @ 20 MHz */
    buttons_init();           /* S1..S4 debounced sampler */
    display_init();           /* blank display */
    buzzer_init();            /* TCA0 WO0 on PB0 */
    adc_init_pot_8bit();      /* free-run 8-bit on POT (AIN2) */
    buzzer_stop();

    /* Seed LFSR with your student number */
    sequencing_init(0x11993251u);

    sei();
    state_machine();
    while (1) { }             /* should never return */
}

/* ---------------- Game state machine ----------------
   Flow:
   - GS_WAIT_START: wait for any S1..S4 (button or UART) to begin
   - GS_PLAYBACK:   extend sequence by one, replay entire sequence (50/50 inside)
   - GS_INPUT:      wait for user presses; compare to sequence (echo ≥ 1/2 delay)
   - GS_FAIL:       show dashes + UART "GAME OVER" + score; then go to WAIT_START
------------------------------------------------------ */
static void state_machine(void)
{
    static uint8_t seq[64];
    static uint8_t len = 0;
    static uint8_t i = 0;
    game_state_t gs = GS_WAIT_START;

    /* Current round’s delay (from POT) */
    static uint16_t current_step_delay_ms = STEP_DELAY_MS;

    uint32_t guard_until = 0;

    /* For immediate INC/DEC while a tone is active */
    uint16_t last_base_hz = 0;   /* base tone currently playing (0 if silent) */
    uint16_t active_hz    = 0;   /* mutable live tone (INC/DEC edits this) */

    while (1) {

        /* Always let UART handle key commands (S-keys inject into buttons FIFO).
           Also handles RESET and SEED parsing. */
        uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);

        /* If we’re idle at WAIT_START, a RESET or pending SEED should apply now */
        if (gs == GS_WAIT_START) {
            if (uart_take_reset_and_clear()) {
                /* Reset PRNG seed to default and clear length */
                sequencing_init(0x11993251u);
                len = 0;
            }
            if (uart_seed_pending()) {
                sequencing_init(uart_take_seed()); /* applies to next/new game */
                len = 0;
            }
        }

        switch (gs) {

        case GS_WAIT_START:
            display_blank();
            buzzer_stop();
            /* accept any queued press instantly (from button or UART) */
            if (buttons_pop() >= 0) {
                len = 0;
                gs = GS_PLAYBACK;
            }
            break;

        case GS_PLAYBACK: {
            /* apply pending SEED only when a new game actually begins */
            if (len == 0 && uart_seed_pending()) {
                sequencing_init(uart_take_seed());
            }

            if (len < sizeof seq) seq[len++] = sequencing_next_step();

            buttons_clear_all();               /* ignore presses during playback */

            /* Sample POT once per round (0.25–2.0 s) */
            uint8_t pot = adc_read8();
            current_step_delay_ms = playback_delay_ms_from_adc8(pot);

            /* Simon playback (50/50 done inside) */
            play_sequence(seq, len, current_step_delay_ms);

            buttons_clear_all();               /* drop anything that happened */

            i = 0;
            guard_until = millis() + INPUT_GUARD_MS;
            gs = GS_INPUT;
            break;
        }

        case GS_INPUT: {
            /* brief guard to ignore latent edges right after playback */
            if ((int32_t)(millis() - guard_until) < 0) {
                buttons_clear_all();
                break;
            }

            int8_t b = buttons_pop();
            if (b < 0) break;  /* no press yet */

            uint8_t expected = seq[i];
            if ((uint8_t)b == expected) {
                /* Show correct bars + play the step tone for ≥ 1/2 delay, or as long as held */
                extern const uint16_t step_freq[4];

                if (b < 2) {
                    display_write_lhs((b == 0) ? (DISP_SEG_E & DISP_SEG_F)
                                               : (DISP_SEG_B & DISP_SEG_C));
                } else {
                    display_write_rhs((b == 2) ? (DISP_SEG_E & DISP_SEG_F)
                                               : (DISP_SEG_B & DISP_SEG_C));
                }

                last_base_hz = step_freq[b];
                active_hz    = uart_apply_freq_offset(last_base_hz);
                buzzer_start_hz(active_hz);

                uint16_t min_on = current_step_delay_ms / 2;
                uint32_t t0 = millis();
                while ((uint32_t)(millis() - t0) < min_on) {
                    /* INC/DEC must retune immediately while buzzer is active */
                    uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);
                }
                while (buttons_is_down((uint8_t)b)) {
                    uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);
                    if ((uint32_t)(millis() - t0) > (uint32_t)(current_step_delay_ms * 2U)) break;
                }

                buzzer_stop();
                last_base_hz = 0; active_hz = 0;
                display_blank();

                i++;
                if (i == len) {
                    /* SUCCESS: “8 8” for one full step delay + UART print */
                    uart_game_success(len);
                    last_base_hz = SUCCESS_TONE_HZ;
                    active_hz    = uart_apply_freq_offset(last_base_hz);
                    buzzer_start_hz(active_hz);

                    uint32_t s0 = millis();
                    while ((uint32_t)(millis() - s0) < current_step_delay_ms) {
                        display_write_lhs(DISP_ON);  delay_ms(1);
                        display_write_rhs(DISP_ON);  delay_ms(1);
                        uart_poll_commands((uint8_t)gs, last_base_hz, &active_hz);
                    }
                    buzzer_stop();
                    last_base_hz = 0; active_hz = 0;
                    display_blank();

                    /* Continue to next round (no explicit win state) */
                    delay_ms(ROUND_PAUSE_MS);
                    gs = GS_PLAYBACK;
                }
            } else {
                gs = GS_FAIL;
            }
            break;
        }
            case GS_FAIL: {
                /* UART message per spec */
                uart_game_over(len);

                /* FAIL pattern: dash-dash for one step delay + fail tone */
                buzzer_start_hz(uart_apply_freq_offset(FAIL_TONE_HZ));
                uint32_t f0 = millis();
                while ((uint32_t)(millis() - f0) < current_step_delay_ms) {
                    display_write_lhs(DISP_DASH); delay_ms(1);
                    display_write_rhs(DISP_DASH); delay_ms(1);
                    /* allow live INC/DEC and keep persistence */
                    uint16_t live = uart_apply_freq_offset(FAIL_TONE_HZ);
                    uart_poll_commands((uint8_t)gs, FAIL_TONE_HZ, &live);
                }
                buzzer_stop();
                display_blank();

                /* Show score in decimal for one delay (you already have this helper) */
                display_score_decimal_for(len, current_step_delay_ms);

                /* Blank for one delay (spec) */
                delay_ms(current_step_delay_ms);
                display_blank();

                /* If score is in top 5 -> prompt for name (name input takes precedence while active) */
                if (highscore_qualifies(len)) {
                    highscore_prompt_and_store(len);
                }

                buttons_clear_all();
                gs = GS_WAIT_START;
                break;
            }

        } /* switch */
    } /* while */
}
