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

static void echo_button(uint8_t b)
{
    extern const uint16_t step_freq[4];
    
    // Echo the button press with tone and pattern
    uint16_t freq = uart_apply_freq_offset(step_freq[b]);
    buzzer_start_hz(freq);
    
    if (b < 2) {
        display_set((b == 0) ? (DISP_SEG_E & DISP_SEG_F) : (DISP_SEG_B & DISP_SEG_C), DISP_OFF);
    } else {
        display_set(DISP_OFF, (b == 2) ? (DISP_SEG_E & DISP_SEG_F) : (DISP_SEG_B & DISP_SEG_C));
    }
    
    delay_ms(125);
    
    buzzer_stop();
    display_blank();
}

static void state_machine(void)
{
    typedef enum { GS_WAIT_START, GS_PLAYBACK, GS_INPUT, GS_FAIL } game_state_t;
    
    static uint32_t round_start_state = 0;
    static uint8_t len = 0;
    static uint8_t i = 0;
    static uint16_t current_step_delay_ms = STEP_DELAY_MS;
    
    game_state_t gs = GS_PLAYBACK;

    while (1) {
        // Handle UART commands
        uart_poll_commands((uint8_t)gs, 0, 0);

        if (gs == GS_WAIT_START) {
            if (uart_take_reset_and_clear()) { 
                sequencing_init(0x11993251u); 
                len = 0; 
            }
            if (uart_seed_pending()) { 
                sequencing_init(uart_take_seed()); 
                len = 0; 
            }
        }

        switch (gs) {
        case GS_WAIT_START:
            display_blank();
            buzzer_stop();
            if (buttons_pop() >= 0) { 
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
            delay_ms(5);  // Brief pause
            gs = GS_INPUT;
            break; }

        case GS_INPUT: {
            int8_t b = buttons_pop();
            if (b < 0) break;
            
            // Check if input matches expected step
            uint32_t keep = sequencing_save_state();
            sequencing_restore_state(round_start_state);
            uint8_t expected = 0;
            for (uint8_t j = 0; j <= i; j++) {
                expected = sequencing_next_step();
            }
            sequencing_restore_state(keep);
            
            // Echo the button press
            echo_button((uint8_t)b);
            
            if ((uint8_t)b == expected) {
                i++;
                if (i == len) {
                    // Success - show pattern and advance
                    uart_game_success(len);
                    display_set(DISP_ON, DISP_ON);
                    delay_ms(current_step_delay_ms);
                    display_blank();
                    delay_ms(ROUND_PAUSE_MS);
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
            delay_ms(current_step_delay_ms);
            buzzer_stop();
            display_blank();
            
            display_score_decimal_for(len, current_step_delay_ms);
            delay_ms(current_step_delay_ms);
            
            if (highscore_qualifies(len)) {
                highscore_prompt_and_store(len);
            }
            buttons_clear_all();
            gs = GS_WAIT_START;
            break; }
        }
    }
}
