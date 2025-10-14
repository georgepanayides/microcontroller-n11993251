#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint16_t elapsed_time = 0;

void clock_init(void)
{
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;
}//clock_init

void led_init(void){
    PORTB.OUTSET = PIN5_bm;  //TURN LED OFF 
    PORTB.DIRSET = PIN5_bm;  //SET PB5 as an output
}//led_init

void timer_init(void) {

    // configure TCB0 for a periodic interrupt every 1ms
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode

    // Set interval for 1ms with 3.333333MHz clock
    // f_cpu = 3.333333MHz, t_CPU = 1/f_CPU = 300ns
    // t_timer = 1ms
    // counts = 1ms / 300ns  = 3333 
    //TCB0.CCMP = 3333;

     // Set interval for 1ms with 10MHz clock
    // f_cpu = 10MHz, t_CPU = 1/f_CPU = 100ns
    // t_timer = 1ms
    // counts = 1ms / 100ns  = 10000 
    TCB0.CCMP = 10000;
  
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable TCB0
}//timer_init

void initialisation (void) {
    cli();      // disable interrupts globally
    clock_init();
    led_init();
    timer_init();  
    sei();      // enable interrupts globally
}//initialisation


int main (void) {
    initialisation();

    while (1) {
        if (elapsed_time == 1000) {  //1s
            PORTB.OUTTGL = PIN5_bm;
            elapsed_time = 0;
        }
    }//while
}//main

// periodic interrupt every 1ms
ISR(TCB0_INT_vect)
{
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;  // Clear the interrupt flag
}//TCB0_INT_vect
