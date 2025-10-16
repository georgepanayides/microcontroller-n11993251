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

static void state_machine(void);

#define FAIL_TONE_HZ 400

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

static void display_score(uint16_t score, uint16_t ms)
{
    extern volatile uint16_t elapsed_time;
    uint8_t show = (uint8_t)(score % 100);
    uint8_t tens = show / 10, ones = show % 10;
    uint8_t left_mask  = (tens == 0 && score < 100) ? DISP_OFF : digit_mask(tens);
    uint8_t right_mask = digit_mask(ones);
    display_set(left_mask, right_mask);
    elapsed_time = 0;
    while (elapsed_time < ms) {}
    display_blank();
}

int main(void)
{
    cli();
    gpio_init();
    spi_init_for_display();
    tcb0_init_1ms();
    // uart_init_9600_8n1();  // DISABLED for debugging
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

static void enable_outputs(uint8_t step_index)
{
    extern const uint16_t step_freq[4];
    
    switch (step_index) {
        case 0: display_set(DISP_BAR_LEFT, DISP_OFF); break;
        case 1: display_set(DISP_BAR_RIGHT, DISP_OFF); break;
        case 2: display_set(DISP_OFF, DISP_BAR_LEFT); break;
        case 3: display_set(DISP_OFF, DISP_BAR_RIGHT); break;
    }
    
    buzzer_start_hz(step_freq[step_index]);
}

static void disable_outputs(void)
{
    buzzer_stop();
    display_blank();
}

static void state_machine(void)
{
    extern const uint16_t step_freq[4];
    typedef enum { GS_WAIT_START, GS_PLAYBACK, GS_INPUT, GS_BUTTON_HELD, GS_FAIL } game_state_t;
    
    static uint32_t round_start_state = 0;
    static uint8_t len = 0;
    static uint8_t i = 0;
    static uint16_t current_step_delay_ms = 1200;
    static uint8_t played_steps[64];
    static uint8_t held_button = 0xFF;
    static uint16_t min_duration = 0;
    
    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_falling, pb_rising;
    extern volatile uint16_t elapsed_time;
    
    game_state_t gs = GS_PLAYBACK;

    while (1) {
        // Simple debouncing like demos
        buttons_debounce();
        
        pb_state_r = pb_state;
        pb_state = buttons_get_debounced_state();
        pb_changed = pb_state_r ^ pb_state;
        pb_falling = pb_changed & pb_state_r;
        pb_rising = pb_changed & pb_state;
        
        switch (gs) {
        case GS_WAIT_START:
            display_blank();
            buzzer_stop();
            if (pb_falling & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) {
                len = 0; 
                gs = GS_PLAYBACK; 
            }
            break;

        case GS_PLAYBACK: {
            // Set up sequence generation for this round
            if (len == 0) {
                // Starting a new game - save current LFSR position as round start
                round_start_state = sequencing_save_state();
            }
            len++;  // Increase sequence length for this round
            
            // Read potentiometer to determine playback speed
            uint8_t pot = adc_read8();
            current_step_delay_ms = playback_delay_ms_from_adc8(pot);
            
            // Generate and play the sequence for this round
            sequencing_restore_state(round_start_state);  // Start from round beginning
            for (uint8_t j = 0; j < len; j++) {
                uint8_t step = sequencing_next_step();  // Get next step from LFSR
                play_step(step, current_step_delay_ms);  // Play tone and show display
                if (j < sizeof(played_steps)) played_steps[j] = step;  // Remember for checking user input
            }
            
            // Prepare for user input phase
            sequencing_restore_state(round_start_state);  // Reset LFSR for next round
            i = 0;  // Reset user input counter
            pb_state = buttons_get_debounced_state();  // Clear button state
            pb_state_r = pb_state;
            gs = GS_INPUT;  // Switch to waiting for user input
            break; }        
            
        case GS_INPUT: {
            // Check which button was just pressed (falling edge detection)
            int8_t b = -1;
            if (pb_falling & PIN4_bm) b = 0;  // S1 pressed → step 0
            else if (pb_falling & PIN5_bm) b = 1;  // S2 pressed → step 1
            else if (pb_falling & PIN6_bm) b = 2;  // S3 pressed → step 2
            else if (pb_falling & PIN7_bm) b = 3;  // S4 pressed → step 3
            
            if (b >= 0) {
                // Check if correct button
                uint8_t expected = (i < sizeof(played_steps)) ? played_steps[i] : 0xFF;
                if ((uint8_t)b == expected) {
                    // Start button output immediately
                    enable_outputs((uint8_t)b);
                    held_button = (uint8_t)b;
                    min_duration = current_step_delay_ms / 2;  // 50% of playback delay
                    elapsed_time = 0;
                    gs = GS_BUTTON_HELD;
                } else {
                    gs = GS_FAIL;
                }
            }
            break; }

        case GS_BUTTON_HELD: {
            uint8_t button_mask = (1 << (held_button + 4));
            
            // Check if button released
            if (pb_rising & button_mask) {
                // Button released - check if minimum duration met
                if (elapsed_time >= min_duration) {
                    disable_outputs();
                    i++;  // Move to next step
                    
                    // Check if sequence complete
                    if (i == len) {
                        display_set(DISP_ON, DISP_ON);
                        elapsed_time = 0;
                        while (elapsed_time < current_step_delay_ms) {}
                        display_blank();
                        gs = GS_PLAYBACK;
                    } else {
                        gs = GS_INPUT;  // Wait for next button
                    }
                } else {
                    // Button released too early - keep output until minimum duration
                    while (elapsed_time < min_duration) {}
                    disable_outputs();
                    i++;
                    
                    if (i == len) {
                        display_set(DISP_ON, DISP_ON);
                        elapsed_time = 0;
                        while (elapsed_time < current_step_delay_ms) {}
                        display_blank();
                        gs = GS_PLAYBACK;
                    } else {
                        gs = GS_INPUT;
                    }
                }
            }
            // If button still held and minimum duration reached, keep output active
            break; }

        case GS_FAIL: {
            // Show fail sequence
            display_set(DISP_DASH, DISP_DASH);  // Show "--" pattern
            buzzer_start_hz(FAIL_TONE_HZ);  // Play fail tone
            elapsed_time = 0;
            while (elapsed_time < current_step_delay_ms) {}  // Hold fail tone/display
            buzzer_stop();
            
            // Show user's score (sequence length they reached)
            display_score(len, current_step_delay_ms);
            display_blank();
            elapsed_time = 0;
            while (elapsed_time < current_step_delay_ms) {}  // Brief pause
            
            // Advance LFSR for next game (as per assignment requirements)
            sequencing_restore_state(round_start_state);  // Go back to start of failed round
            for (uint8_t j = 0; j < len; j++) {
                sequencing_next_step();  // Advance past the sequence we just played
            }
            // The LFSR is now positioned at the next step for the new game
            
            len = 0;  // Reset sequence length for new game
            display_blank();  // Ensure display is clear before new round
            gs = GS_PLAYBACK;  // Start new game immediately
            break; }
        }
    }
}
