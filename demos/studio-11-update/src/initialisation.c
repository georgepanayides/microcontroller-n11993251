#include "initialisation.h"

#include <avr/io.h>

void buttons_init(void)
{
    // Enable pull-up resistors for pushbuttons
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void port_init(void)
{
    // BUZZER (PIN0), USART0 TXD (PIN2)
    PORTB.DIRSET = PIN0_bm | PIN2_bm;
}

void pwm_init(void)
{
    // Enable output override on PB0.
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // PWM initially OFF
    TCA0.SINGLE.PER = 1;
    TCA0.SINGLE.CMP0 = 0;

    // Enable TCA0
    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
}

void timers_init(void)
{
    // 1ms interrupt for elapsed time
    TCB0.CCMP = 3333;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;

    // 5ms interrupt for pushbutton sampling
    TCB1.CCMP = 16667;
    TCB1.INTCTRL = TCB_CAPT_bm;
    TCB1.CTRLA = TCB_ENABLE_bm;
}

void uart_init(void)
{
    // 9600 baud
    USART0.BAUD = 1389;

    // Enable receive complete interrupt
    USART0.CTRLA = USART_RXCIE_bm;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}
