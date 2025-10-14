#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "timer.h"
#include "pushbutton.h"
#include "buzzer.h"
#include "adc.h"
#include "display.h"
#include "uart.h" 

#define MIN_PLAYBACK_DELAY 1000
#define MAX_PLAYBACK_DELAY 3000

extern uint8_t pb_debounced;
extern uint16_t elapsed_time;

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

int main (void) {  
    initialisation();

    uint8_t pb_state = 0xFF;
    uint8_t pb_state_r = 0xFF;
    uint8_t pb_changed, pb_rising, pb_falling;

    typedef enum {
        STATE_WAITING,
        STATE_ZERO,  
        STATE_ONE,
        STATE_TWO,
        STATE_THREE  
    } Game_State;

    Game_State state = STATE_WAITING;
    uint16_t playback_delay = MIN_PLAYBACK_DELAY;

    stop_tone();  
    //display_write(SEGS_OFF);       
    //display_write(SEGS_OFF | (0x01 << 7));
    set_display_segments(SEGS_OFF, SEGS_OFF);

    while (1) {
        pb_state_r = pb_state;      // register the previous pushbutton sample
        //pb_state = PORTA.IN;        // new sample of current pushbutton state
        pb_state = pb_debounced;    // new sample of current pushbutton state - after debouncing

        pb_changed = pb_state_r ^ pb_state;    

        pb_falling = pb_changed & pb_state_r;   
        pb_rising = pb_changed & pb_state;

        playback_delay = (((uint16_t) (MAX_PLAYBACK_DELAY - MIN_PLAYBACK_DELAY) * ADC0.RESULT) >> 8) + MIN_PLAYBACK_DELAY;

        switch (state) {
            case STATE_WAITING:
                // S1 released
                if (pb_rising & PIN4_bm) {       
                    //printf("0 "); 
                    play_tone(0);        
                    //display_write(SEGS_BARR);
                    set_display_segments(SEGS_OFF, SEGS_BARR);                     
                    state = STATE_ZERO;
                    elapsed_time = 0;
                }
                break;

            case STATE_ZERO:
                if (elapsed_time > playback_delay) {                     
                    printf("1 "); 
                    play_tone(1);       
                    //display_write(SEGS_BARR & SEGS_BARL); 
                    set_display_segments(SEGS_OFF, SEGS_BARR & SEGS_BARL);                     
                    state = STATE_ONE;              
                    elapsed_time = 0;
                }
                break;

            case STATE_ONE:
                if (elapsed_time > playback_delay) {
                    //printf("2 "); 
                    play_tone(2);     
                    //display_write(SEGS_BARR | (0x01 << 7));  
                    set_display_segments(SEGS_BARR, SEGS_BARR & SEGS_BARL);                    
                    state = STATE_TWO;                    
                    elapsed_time = 0;
                }
                break;

            case STATE_TWO:
                if (elapsed_time > playback_delay) {
                    //printf("3 ");
                    play_tone(3);    
                    //display_write((SEGS_BARR & SEGS_BARL) | (0x01 << 7));
                    set_display_segments(SEGS_BARR & SEGS_BARL, SEGS_BARR & SEGS_BARL);
                    state = STATE_THREE;                    
                    elapsed_time = 0;
                }
                break;

            case STATE_THREE:
                if (elapsed_time > playback_delay) {
                    stop_tone();
                    //display_write(SEGS_OFF);
                    //display_write(SEGS_OFF | (0x01 << 7));
                    set_display_segments(SEGS_OFF, SEGS_OFF);
                    state = STATE_WAITING;
                }
                break;

             default:
                stop_tone();    
                //display_write(SEGS_OFF);                
                //display_write(SEGS_OFF | (0x01 << 7));    
                set_display_segments(SEGS_OFF, SEGS_OFF);                         
                state = STATE_WAITING;
        }//switch
    }//while
}//main