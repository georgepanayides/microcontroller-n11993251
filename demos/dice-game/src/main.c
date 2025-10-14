#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "timer.h"
#include "pushbutton.h"
#include "buzzer.h"
#include "adc.h"
#include "display.h"
#include "uart.h"
#include "dice.h"
#include "game_state.h"    

#define MIN_PLAYBACK_DELAY 500
#define MAX_PLAYBACK_DELAY 2500

extern uint8_t pb_debounced;
extern uint16_t elapsed_time;
//extern uint8_t octave;  //debugging

volatile Game_State state = AWAITING_INPUT;

Dice d1;
Dice d2;

volatile uint8_t result_d1;
volatile uint8_t result_d2;

volatile uint8_t uart_roll = 0;
volatile uint8_t digits[3] = {0,0,0};

uint16_t dice_results[11] = {0,0,0,0,0,0,0,0,0,0,0}; //reults for score 2-12

void initialisation (void) {
    cli();      // disable interrupts globally
    pb_init();
    timer_init();    
    buzzer_init();
    adc_init();
    display_init(); 
    uart_init();
    sei();      // enable interrupts globally
}//initialisation

void print_scores(uint16_t *arr) {
    printf("\n");
    for (uint8_t i=0; i < 11; i++) 
        printf("%2u: %u\n", i+2, *(arr+i));    
    
    printf("\n");        
}//print_scores

uint16_t calc_decimal()
{ 
    uint16_t result=0;

    for (uint8_t i=0; i < digits[2]; i++)
       result += 100;

    for (uint8_t i=0; i < digits[1]; i++)
       result += 10;

    result += digits[0];       

    return result;
}//calc_decimal

void roll_n_times(uint16_t num_rolls) {
    for (uint16_t n=0; n < num_rolls; n++)
    {
        result_d1 = roll_dice (&d1);
        result_d2 = roll_dice (&d2);          

        uint8_t score = result_d1 + result_d2; 
        dice_results[score - 2]++;
    }//for

}//roll_n_times

int main (void) {  
    initialisation();

    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_rising, pb_falling;

    uint16_t count = 0;
    uint8_t digit2, digit1, digit0;    
    uint8_t score;
    uint16_t num_rolls;
 
    uint16_t playback_delay = MIN_PLAYBACK_DELAY;

    init_dice (&d1,0x00CAB202);
    init_dice (&d2,0x12345678);

    while (1) {
        pb_state_r = pb_state;      // register the previous pushbutton sample
        //pb_state = PORTA.IN;        // new sample of current pushbutton state
        pb_state = pb_debounced;    // new sample of current pushbutton state - after debouncing

        pb_changed = pb_state_r ^ pb_state;    

        pb_falling = pb_changed & pb_state_r;   
        pb_rising = pb_changed & pb_state;

        // if S1 pressed
        //if (pb_falling & PIN4_bm) {
        //    printf ("%u\n",count);
        //    count = (count == 99)? 0 : count + 1;            
        //}

        playback_delay = (((uint16_t) (MAX_PLAYBACK_DELAY - MIN_PLAYBACK_DELAY) * ADC0.RESULT) >> 8) + MIN_PLAYBACK_DELAY;

        switch (state) {
            case AWAITING_INPUT:
                // S1 pressed
                if (pb_falling & PIN4_bm) {
                    result_d1 = roll_dice (&d1);
                    result_d2 = roll_dice (&d2);                 
                    state = ROLL_DICE;

                // S2 pressed                    
                } else if (pb_falling & PIN5_bm) {
                    state = OUTPUT_SCORES;                    
                }
                break;

            case ROLL_DICE:
                // S1 releaseed
                if ((pb_rising & PIN4_bm) || uart_roll) {
                    uart_roll = 0;

                    count++;
                    printf("%3u: %u %u ", count, result_d1, result_d2);     
                    
                    set_display_numbers(result_d1, result_d2);
                    state = DISPLAY_DICE;
                    elapsed_time = 0;
                }
                break;

            case DISPLAY_DICE:
                if (elapsed_time > playback_delay) {
                    score = result_d1 + result_d2; 
                    dice_results[score - 2]++;
                    printf("%2u",score);  

                    uint8_t tone = (score < 7)? 0 : ((score > 7)? 2 : 1);
                    play_tone (tone);
                    //printf (" %d ", octave); //debugging

                    find_dec_digits(score, &digit2, &digit1, &digit0);

                    state = DISPLAY_SCORE;
                    elapsed_time = 0;
                }
                break;                    

            case DISPLAY_SCORE:
                if (elapsed_time > playback_delay) {

                    if (score < 7)
                        set_display_segments(SEGS_ALL, SEGS_BAR);
                    else if (score > 7)
                        set_display_segments(SEGS_BAR, SEGS_ALL);
                    else
                        set_display_segments(SEGS_ALL, SEGS_ALL);

                    state = DISPLAY_PATTERN;
                    elapsed_time = 0;                    
                }
                break; 

            case DISPLAY_PATTERN:
                if (elapsed_time > playback_delay) {
                    printf("\n");    
                    stop_tone();     
                    set_display_segments(SEGS_OFF, SEGS_OFF);             
                    state = AWAITING_INPUT;
                }
                break; 

            case OUTPUT_SCORES:
                print_scores(dice_results);            
                state = AWAITING_INPUT;
                break; 

            case ROLL_N_TIMES:
                num_rolls = calc_decimal();            
                roll_n_times(num_rolls);            
                state = AWAITING_INPUT;
                break;               

             default:
                state = AWAITING_INPUT;
        }//switch
    }//while
}//main