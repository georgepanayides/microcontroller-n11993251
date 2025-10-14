#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "qutyserial.h"
#include "timer.h"
#include "pushbutton.h"
#include "buzzer.h"
#include "adc.h"
#include "dice.h"

#define MIN_PLAYBACK_DELAY 1000
#define MAX_PLAYBACK_DELAY 3000

extern uint8_t pb_debounced;
extern uint16_t elapsed_time;

typedef enum {
    AWAITING_INPUT,
    ROLL_DICE,
    DISPLAY_DICE,
    DISPLAY_SCORE,
    OUTPUT_SCORES
} Game_State;

void initialisation (void) {
    cli();      // disable interrupts globally
    pb_init();
    timer_init();    
    buzzer_init();
    adc_init();
    sei();      // enable interrupts globally
}//initialisation

void print_scores(uint16_t *arr){
    printf("\n");
    for (uint8_t i=0; i < 11; i++) 
        printf("%2u: %u\n", i+2, *(arr+i));    

    printf("\n");        
}//print_scores

int main (void) {  
    serial_init();
    initialisation();

    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_rising, pb_falling;

    uint16_t count = 0;
 
    Game_State state = AWAITING_INPUT;

    uint16_t playback_delay = MIN_PLAYBACK_DELAY;

    Dice d1, d2;
    uint8_t result_d1, result_d2, score;
    uint16_t dice_results[11] = {0,0,0,0,0,0,0,0,0,0,0}; //reults for score 2-12

    init_dice (&d1,0x00CAB202);
    init_dice (&d2,0x12345678);

    while (1) {
        pb_state_r = pb_state;     // register the previous pushbutton sample
        //pb_state = PORTA.IN;       // new sample of current pushbutton state
        pb_state = pb_debounced;    // new sample of current pushbutton state - after debouncing

        pb_changed = pb_state_r ^ pb_state;    

        pb_falling = pb_changed & pb_state_r;   
        pb_rising = pb_changed & pb_state;

        // if S1 pressed
        //if (pb_falling & PIN4_bm) {
        //    printf ("%u\n",count);
        //    count = (count == 99)? 0 : count + 1;            
        //}

        switch (state) {
            case AWAITING_INPUT:
                // S1 pressed
                if (pb_falling & PIN4_bm) {
                    result_d1 = roll_dice (&d1);
                    result_d2 = roll_dice (&d2);                 
                    state = ROLL_DICE;

                // S2 pressed                    
                } else if (pb_falling & PIN5_bm) {
                    print_scores(dice_results);
                    state = OUTPUT_SCORES;                    
                }
                break;

            case ROLL_DICE:
                // S1 releaseed
                if (pb_rising & PIN4_bm) {
                    count++;
                    printf("%3u: %u %u ", count, result_d1, result_d2);                    
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

                    state = DISPLAY_SCORE;
                    elapsed_time = 0;
                }
                break;                    

            case DISPLAY_SCORE:
                if (elapsed_time > playback_delay) {
                    printf("\n");    
                    stop_tone();                  
                    state = AWAITING_INPUT;
                }
                break; 

            case OUTPUT_SCORES:
                state = AWAITING_INPUT;
                break; 

             default:
                state = AWAITING_INPUT;
        }//switch
    }//while
}//main