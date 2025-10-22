#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "timer.h"
#include "buttons.h"
#include "buzzer.h"
#include "adc.h"
#include "display.h"
#include "display_macros.h"
#include "uart.h"
#include "sequencing.h"

#define MIN_PLAYBACK_DELAY 250
#define MAX_PLAYBACK_DELAY 2000
#define FAIL_TONE_HZ 400

extern uint8_t pb_debounced;
extern volatile uint16_t elapsed_time;

// Simon game variables
static uint32_t round_start_state = 0;
static uint8_t len = 0;
static uint8_t i = 0;
static uint8_t played_steps[64];
static uint8_t pb_step_index = 0;

void initialisation (void) {
    cli();
    buttons_init();
    timer_init();    
    buzzer_init();
    adc_init();
    display_init(); 
    uart_init();
    sequencing_init(0x11993251u);
    sei();
}//initialisation

int main (void) {  
    initialisation();

    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_rising, pb_falling;

    typedef enum {
        PLAYBACK_START,
        PLAYBACK_STEP_ON,
        PLAYBACK_STEP_OFF,
        INPUT_WAITING,
        INPUT_ECHO_ON,
        SUCCESS_SHOW,
        FAIL_SHOW,
        FAIL_SCORE_SHOW,
        FAIL_WAIT
    } Game_State;

    Game_State state = PLAYBACK_START;
    uint16_t playback_delay = MIN_PLAYBACK_DELAY;
    int8_t input_button = -1;

    buzzer_stop();
    set_display_segments(DISP_OFF, DISP_OFF);

    while (1) {
        pb_state_r = pb_state;      // register the previous pushbutton sample
        pb_state = PORTA.IN;        // new sample of current pushbutton state
        pb_state = pb_debounced;    // new sample of current pushbutton state - after debouncing

        pb_changed = pb_state_r ^ pb_state;    

        pb_falling = pb_changed & pb_state_r;   
        pb_rising = pb_changed & pb_state;

        playback_delay = (((uint16_t) (MAX_PLAYBACK_DELAY - MIN_PLAYBACK_DELAY) * ADC0.RESULT) >> 8) + MIN_PLAYBACK_DELAY;

        switch (state) {
            case PLAYBACK_START:
                // Start new round
                if (len == 0) round_start_state = sequencing_save_state();
                len++;
                
                // Use sequencing helper function instead of loop
                sequencing_generate_sequence(round_start_state, len, played_steps);
                
                pb_step_index = 0;
                buzzer_on(played_steps[0]);
                set_display_segments(left_patterns[played_steps[0]], right_patterns[played_steps[0]]);
                state = PLAYBACK_STEP_ON;
                elapsed_time = 0;
                break;

            case PLAYBACK_STEP_ON:
                if (elapsed_time > (playback_delay >> 1)) {  // 50% of playback delay
                    buzzer_stop();
                    set_display_segments(DISP_OFF, DISP_OFF);
                    state = PLAYBACK_STEP_OFF;
                    elapsed_time = 0;
                }
                break;

            case PLAYBACK_STEP_OFF:
                if (elapsed_time > (playback_delay >> 1)) {
                    pb_step_index++;
                    if (pb_step_index >= len) {
                        // Playback done, wait for input
                        i = 0;
                        pb_state = pb_debounced;
                        pb_state_r = pb_state;
                        uart_game_input = -1;
                        uart_input_enabled = 1;
                        state = INPUT_WAITING;
                    } else {
                        buzzer_on(played_steps[pb_step_index]);
                        set_display_segments(left_patterns[played_steps[pb_step_index]], right_patterns[played_steps[pb_step_index]]);
                        state = PLAYBACK_STEP_ON;
                        elapsed_time = 0;
                    }
                }
                break;

            case INPUT_WAITING:
                // Check UART first
                if (uart_game_input >= 0) {
                    input_button = uart_game_input;
                    uart_game_input = -1;
                    buzzer_on((uint8_t)input_button);
                    set_display_segments(left_patterns[input_button], right_patterns[input_button]);
                    state = INPUT_ECHO_ON;
                    elapsed_time = 0;
                // Then check pushbuttons
                } else if (pb_falling & PIN4_bm) {
                    input_button = 0;
                    buzzer_on(0);
                    set_display_segments(left_patterns[0], right_patterns[0]);
                    state = INPUT_ECHO_ON;
                    elapsed_time = 0;
                } else if (pb_falling & PIN5_bm) {
                    input_button = 1;
                    buzzer_on(1);
                    set_display_segments(left_patterns[1], right_patterns[1]);
                    state = INPUT_ECHO_ON;
                    elapsed_time = 0;
                } else if (pb_falling & PIN6_bm) {
                    input_button = 2;
                    buzzer_on(2);
                    set_display_segments(left_patterns[2], right_patterns[2]);
                    state = INPUT_ECHO_ON;
                    elapsed_time = 0;
                } else if (pb_falling & PIN7_bm) {
                    input_button = 3;
                    buzzer_on(3);
                    set_display_segments(left_patterns[3], right_patterns[3]);
                    state = INPUT_ECHO_ON;
                    elapsed_time = 0;
                }
                break;

            case INPUT_ECHO_ON:
                if ((pb_rising & (1 << (input_button + 4))) || (elapsed_time > (playback_delay >> 1))) {
                    buzzer_stop();
                    set_display_segments(DISP_OFF, DISP_OFF);
                    
                    if ((uint8_t)input_button == played_steps[i]) {
                        i++;
                        if (i == len) {
                            uart_input_enabled = 0;
                            set_display_segments(DISP_ON, DISP_ON);
                            state = SUCCESS_SHOW;
                            elapsed_time = 0;
                        } else {
                            state = INPUT_WAITING;
                        }
                    } else {
                        uart_input_enabled = 0;
                        state = FAIL_SHOW;
                        elapsed_time = 0;
                    }
                }
                break;

            case SUCCESS_SHOW:
                if (elapsed_time > playback_delay) {
                    set_display_segments(DISP_OFF, DISP_OFF);
                    state = PLAYBACK_START;
                }
                break;

            case FAIL_SHOW:
                if (elapsed_time == 0) {
                    set_display_segments(DISP_DASH, DISP_DASH);
                    buzzer_start_hz(FAIL_TONE_HZ);
                }
                if (elapsed_time > playback_delay) {
                    buzzer_stop();
                    
                    uint8_t show = len % 100;
                    uint8_t tens = show / 10, ones = show % 10;
                    uint8_t left_mask = (tens == 0 && len < 100) ? DISP_OFF : digit_masks[tens];
                    set_display_segments(left_mask, digit_masks[ones]);
                    
                    state = FAIL_SCORE_SHOW;
                    elapsed_time = 0;
                }
                break;

            case FAIL_SCORE_SHOW:
                if (elapsed_time > playback_delay) {
                    set_display_segments(DISP_OFF, DISP_OFF);
                    state = FAIL_WAIT;
                    elapsed_time = 0;
                }
                break;

            case FAIL_WAIT:
                if (elapsed_time > playback_delay) {
                    // Advance LFSR past the failed sequence
                    sequencing_restore_state(round_start_state);
                    for (uint8_t j = 0; j < len; j++) sequencing_next_step();
                    len = 0;
                    state = PLAYBACK_START;
                }
                break;

             default:
                buzzer_stop();
                set_display_segments(DISP_OFF, DISP_OFF);
                state = PLAYBACK_START;
        }//switch
    }//while
}//main