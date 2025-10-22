// Simon Says game for QUTy microcontroller
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

#define FAIL_TONE_HZ 400

// Global state

// studio/tutorial 9 has a state machine we are essentially following and implementing that 
// focus on doing highscore before UART and octave 
// while loops should be removed, because they are blocking, we want non blocking code 

static uint32_t round_start_state = 0;
static uint8_t len = 0;
static uint8_t i = 0;
static uint8_t played_steps[64];
static uint16_t playback_delay_ms = 250;

static const uint8_t digit_masks[10] = {
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_E & DISP_SEG_F,  // 0
    DISP_SEG_B & DISP_SEG_C,                                                        // 1
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_G & DISP_SEG_E & DISP_SEG_D,                // 2
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_G,                // 3
    DISP_SEG_F & DISP_SEG_G & DISP_SEG_B & DISP_SEG_C,                             // 4
    DISP_SEG_A & DISP_SEG_F & DISP_SEG_G & DISP_SEG_C & DISP_SEG_D,                // 5
    DISP_SEG_A & DISP_SEG_F & DISP_SEG_E & DISP_SEG_D & DISP_SEG_C & DISP_SEG_G,   // 6
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C,                                          // 7
    DISP_ON,                                                                        // 8
    DISP_SEG_A & DISP_SEG_B & DISP_SEG_C & DISP_SEG_D & DISP_SEG_F & DISP_SEG_G   // 9
};

static inline void display_score(uint16_t score, uint16_t ms)
{
    extern volatile uint16_t elapsed_time;
    uint8_t show = score % 100;
    uint8_t tens = show / 10, ones = show % 10;
    uint8_t left_mask = (tens == 0 && score < 100) ? DISP_OFF : digit_masks[tens];
    display_set(left_mask, digit_masks[ones]);
    elapsed_time = 0;
    while (elapsed_time < ms) {}
    display_blank();
}

static inline void enable_outputs(uint8_t step_index)
{
    static const uint8_t left_patterns[4] = {DISP_BAR_LEFT, DISP_BAR_RIGHT, DISP_OFF, DISP_OFF};
    static const uint8_t right_patterns[4] = {DISP_OFF, DISP_OFF, DISP_BAR_LEFT, DISP_BAR_RIGHT};
    display_set(left_patterns[step_index], right_patterns[step_index]);
    buzzer_on(step_index);
}

static inline void disable_outputs(void)
{
    buzzer_stop();
    display_blank();
}

static void echo_button(uint8_t b, uint16_t min_duration_ms)
{
    extern volatile uint16_t elapsed_time;
    enable_outputs(b);
    elapsed_time = 0;
    while (elapsed_time < min_duration_ms) {}
    uint8_t button_mask = (1 << (b + 4));
    while ((buttons_get_debounced_state() & button_mask) == 0) {}
    disable_outputs();
}

int main(void)
{
    cli();
    gpio_init();
    spi_init_for_display();
    uart_init();
    buttons_init();
    display_init();
    buzzer_init();
    adc_init();
    buzzer_stop();
    sequencing_init(0x11993251u);
    sei();
    
    // State machine
    typedef enum { GS_PLAYBACK, GS_INPUT, GS_FAIL } game_state_t;
    game_state_t gs = GS_PLAYBACK;
    uint8_t pb_state = 0xFF, pb_state_r = 0xFF, pb_falling;
    
    while (1) {
        // Update button state
        pb_state_r = pb_state;
        pb_state = buttons_get_debounced_state();
        pb_falling = (pb_state_r ^ pb_state) & pb_state_r;
        
        switch (gs) {
        case GS_PLAYBACK:
            if (len == 0) round_start_state = sequencing_save_state();
            len++;
            
            // Read ADC and play sequence
            playback_delay_ms = playback_delay_ms_from_adc8(adc_read8());
            sequencing_restore_state(round_start_state);
            for (uint8_t j = 0; j < len; j++) {
                played_steps[j] = sequencing_next_step();
                play_step(played_steps[j], playback_delay_ms);
            }
            
            // Prepare for input
            i = 0;
            pb_state = buttons_get_debounced_state();
            pb_state_r = pb_state;
            uart_game_input = -1;
            uart_input_enabled = 1;
            gs = GS_INPUT;
            break;
            
        case GS_INPUT: {
            int8_t b = -1;
            if (uart_game_input >= 0) {
                b = uart_game_input;
                uart_game_input = -1;
            }
            
            // Then check buttons if no UART input
            if (b < 0) {
                if (pb_falling & PIN4_bm) b = 0;
                else if (pb_falling & PIN5_bm) b = 1;
                else if (pb_falling & PIN6_bm) b = 2;
                else if (pb_falling & PIN7_bm) b = 3;
                else break; // No input, continue looping
            }
            
            // Echo the input
            echo_button(b, playback_delay_ms >> 1);
            
            // Check if correct
            if ((uint8_t)b == played_steps[i]) {
                i++;
                if (i == len) {
                    // Disable input and show success
                    cli();
                    uart_input_enabled = 0;
                    sei();
                    display_set(DISP_ON, DISP_ON);
                    elapsed_time = 0;
                    while (elapsed_time < playback_delay_ms) {}
                    display_blank();
                    gs = GS_PLAYBACK;
                }
            } else {
                // Incorrect input - disable and fail
                cli();
                uart_input_enabled = 0;
                sei();
                gs = GS_FAIL;
            }
            break; }
            
        case GS_FAIL:
            playback_delay_ms = playback_delay_ms_from_adc8(adc_read8());
            
            // Show fail animation
            display_set(DISP_DASH, DISP_DASH);
            buzzer_start_hz(FAIL_TONE_HZ);
            elapsed_time = 0;
            while (elapsed_time < playback_delay_ms) {}
            buzzer_stop();
            
            // Show score
            display_score(len, playback_delay_ms);
            elapsed_time = 0;
            while (elapsed_time < playback_delay_ms) {} // remove non blocking 
            
            // Advance RNG and reset
            sequencing_restore_state(round_start_state);
            for (uint8_t j = 0; j < len; j++) sequencing_next_step();
            len = 0;
            gs = GS_PLAYBACK;
            break;
        }
    }
}
