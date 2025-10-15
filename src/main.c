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
#include "highscore.h"

// UART button simulation flags
volatile uint8_t uart_s1 = 0, uart_s2 = 0, uart_s3 = 0, uart_s4 = 0;

static void state_machine(void);

#define STEP_DELAY_MS     1200
#define ROUND_PAUSE_MS    1200
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

static void echo_button(uint8_t b, uint16_t playback_delay_ms)
{
    extern const uint16_t step_freq[4];
    extern volatile uint16_t elapsed_time;
    
    // Start echo immediately
    uint16_t freq = uart_apply_freq_offset(step_freq[b]);
    buzzer_start_hz(freq);
    
    if (b < 2) {
        display_set((b == 0) ? (DISP_SEG_E & DISP_SEG_F) : (DISP_SEG_B & DISP_SEG_C), DISP_OFF);
    } else {
        display_set(DISP_OFF, (b == 2) ? (DISP_SEG_E & DISP_SEG_F) : (DISP_SEG_B & DISP_SEG_C));
    }
    
    // Calculate minimum duration (50% of playback delay)
    uint16_t min_duration = playback_delay_ms / 2;
    elapsed_time = 0;
    while (elapsed_time < min_duration) {}
    
    buzzer_stop();
    display_blank();
}

static void state_machine(void)
{
    extern volatile uint16_t elapsed_time;
    typedef enum { GS_WAIT_START, GS_PLAYBACK, GS_INPUT, GS_FAIL } game_state_t;
    
    static uint32_t round_start_state = 0;
    static uint8_t len = 0;
    static uint8_t i = 0;
    static uint16_t current_step_delay_ms = STEP_DELAY_MS;
    
    // Button edge detection variables
    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_falling;
    
    game_state_t gs = GS_PLAYBACK;

    while (1) {
        // Button edge detection 
        pb_state_r = pb_state;
        pb_state = buttons_get_debounced_state();
        pb_changed = pb_state_r ^ pb_state;
        pb_falling = pb_changed & pb_state_r;
        
        switch (gs) {
        case GS_WAIT_START:
            display_blank();
            buzzer_stop();
            // Check for any button press (falling edge) or UART command 
            if ((pb_falling & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) || 
                uart_s1 || uart_s2 || uart_s3 || uart_s4) { 
                // Clear UART flags
                uart_s1 = uart_s2 = uart_s3 = uart_s4 = 0;
                len = 0; 
                gs = GS_PLAYBACK; 
            }
            break;

        case GS_PLAYBACK: {
            if (len == 0 && uart_seed_pending()) {
                sequencing_init(uart_take_seed());
            }
            if (len == 0) {
                round_start_state = sequencing_save_state();
            }
            len++;
            
            // Get playback delay from pot
            uint8_t pot = adc_read8();
            current_step_delay_ms = playback_delay_ms_from_adc8(pot);
            
            // Play Simon sequence
            sequencing_restore_state(round_start_state);
            for (uint8_t j = 0; j < len; j++) {
                uint8_t step = sequencing_next_step();
                play_step(step, current_step_delay_ms);
            }
            
            i = 0;
            elapsed_time = 0;
            while (elapsed_time < 5) {}  // Brief pause
            gs = GS_INPUT;
            break; }

        case GS_INPUT: {
            // Check for button press (falling edge) or UART command
            int8_t b = -1;
            if ((pb_falling & PIN4_bm) || uart_s1) { b = 0; uart_s1 = 0; }
            else if ((pb_falling & PIN5_bm) || uart_s2) { b = 1; uart_s2 = 0; }
            else if ((pb_falling & PIN6_bm) || uart_s3) { b = 2; uart_s3 = 0; }
            else if ((pb_falling & PIN7_bm) || uart_s4) { b = 3; uart_s4 = 0; }
            
            if (b < 0) break;
            
            // Check if input matches expected step
            uint32_t keep = sequencing_save_state();
            sequencing_restore_state(round_start_state);
            uint8_t expected = 0;
            for (uint8_t j = 0; j <= i; j++) {
                expected = sequencing_next_step();
            }
            sequencing_restore_state(keep);
            
            // Calculate playback delay based on current level and potentiometer
            uint8_t pot = adc_read8();
            uint16_t playback_delay = (uint16_t)(250 + ((pot * 1750UL) / 255));
            
            // Echo the button press with proper duration tracking
            echo_button((uint8_t)b, playback_delay);
            
            if ((uint8_t)b == expected) {
                i++;
                if (i == len) {
                    // Success 
                    uart_game_success(len);
                    display_set(DISP_ON, DISP_ON);
                    elapsed_time = 0;
                    while (elapsed_time < current_step_delay_ms) {}
                    display_blank();
                    elapsed_time = 0;
                    while (elapsed_time < ROUND_PAUSE_MS) {}
                    gs = GS_PLAYBACK;
                }
            } else {
                gs = GS_FAIL;
            }
            break; }

        case GS_FAIL: {
            // Show fail pattern and score
            uart_game_over(len);
            display_set(DISP_DASH, DISP_DASH);
            buzzer_start_hz(FAIL_TONE_HZ);
            elapsed_time = 0;
            while (elapsed_time < current_step_delay_ms) {}
            buzzer_stop();
            display_blank();
            
            display_score_decimal_for(len, current_step_delay_ms);
            elapsed_time = 0;
            while (elapsed_time < current_step_delay_ms) {}
            
            if (highscore_qualifies(len)) {
                highscore_prompt_and_store(len);
            }
            // Clear any pending button state when returning to start
            gs = GS_WAIT_START;
            break; }
        }
    }
}
