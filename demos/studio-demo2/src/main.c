#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "uart.h"
#include "timer.h"
#include "display.h"

#define LED_ON  PORTB.OUTCLR = PIN5_bm  //TURN LED ON 
#define LED_OFF PORTB.OUTSET = PIN5_bm  //TURN LED OFF

void led_init(void) {
    PORTB.OUTSET = PIN5_bm;  //TURN LED OFF 
    PORTB.DIRSET = PIN5_bm;  //SET PB5 as an output
}//led_init

void initialisation (void) {
    cli();      // disable interrupts globally
    led_init();
    timer_init();
    uart_init();
    display_init();    
    sei();      // enable interrupts globally
}//initialisation

int main (void) {  
    initialisation();

    uint8_t count = 0;
    uint8_t digit2, digit1, digit0;
    uint8_t disp_mode = 0; //decimal or hex

    find_dec_digits(count, &digit2, &digit1, &digit0);

    int temp;
    printf ("Enter number: ");
    scanf ("%d",&temp);
    printf ("\nNum entered = %d\n",temp);

    for (;;) {
        uint8_t c = uart_getc(); 
        uart_putc(c);
        uart_putc(' ');

        if (c == 'a') {            
            count += 1;
        }
        else if (c == 's') {
            count -= 1; 
        }
        else if (c == 'd') {
            disp_mode = !disp_mode; 
        }        

        if (disp_mode) {
            find_hex_digits(count, &digit1, &digit0);
        }
        else {
            find_dec_digits(count, &digit2, &digit1, &digit0);
            uart_putc(((digit2>=0) && (digit2<=9))? digit2+'0' : digit2+'A'-10);  
        }
      
        uart_putc(((digit1>=0) && (digit1<=9))? digit1+'0' : digit1+'A'-10);
        uart_putc(((digit0>=0) && (digit0<=9))? digit0+'0' : digit0+'A'-10);
        //uart_putc('\n');      
        
        printf (" Count = %u %x\n",count,count);

        if (count == 10) {
            printf("We are here 1 %u\n\n", count);
            LED_ON;
        
        } else if (count == 20) {
            printf("We are here 2 %u\n\n", count);
            LED_OFF;
        }
    }//for
}//main