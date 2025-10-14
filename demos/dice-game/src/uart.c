#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"
#include "dice.h"
#include "buzzer.h"
#include "game_state.h"

extern Game_State state;
extern Dice d1;
extern Dice d2;
extern uint8_t result_d1;
extern uint8_t result_d2;

extern uint8_t uart_roll;
extern uint8_t octave;
extern uint8_t digits[3];

static int stdio_putchar(char c, FILE *stream);
//static int stdio_getchar(FILE *stream);

static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, NULL, _FDEV_SETUP_RW);

typedef enum {
    AWAITING_COMMAND,
    AWAITING_PAYLOAD
} Serial_State;

void uart_init()
{
    PORTB.DIRSET = PIN2_bm; // enable PB2 as an output (UART TX)

    USART0.BAUD = 1389;                           // 9600 BAUD @ 3.33MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // enable TX/RX

    USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt

    stdout = &stdio;
//    stdin = &stdio;
}

//uint8_t uart_getc(void)
//{
    // while (!(USART0.STATUS & USART_RXCIF_bm));   //wait for receive complete
    // return USART0.RXDATAL;
//}

void uart_putc(uint8_t c)
{
    while (!(USART0.STATUS & USART_DREIF_bm)); // wait for data register empty
    USART0.TXDATAL = c;
}

static int stdio_putchar(char c, FILE *stream)
{
    uart_putc(c);
    return c; // the putchar function must return the character written to the stream
}

//static int stdio_getchar(FILE *stream)
//{
    //return uart_getc();
//}


                    
ISR(USART0_RXC_vect) 
{
    uint8_t rx_data, curr_data, curr_digit;
    static uint8_t digit_num=2;

    //rx_buff[buff_idx] = USART0.RXDATAL;
    //if (buff_idx < BUFF_SIZE)
    //    buff_idx++;
    //        char rx_data = USART0.RXDATAL;

    static Serial_State ser_state = AWAITING_COMMAND;

    rx_data = USART0.RXDATAL;

    //printf("%c ",rx_data);  //debugging

    switch (ser_state)
    {
        case AWAITING_COMMAND:
            if (rx_data == ',')
                decrease_octave();
            else if (rx_data == '.')
                increase_octave();

            if (state == AWAITING_INPUT) {
                if (rx_data == 'r') {
                    result_d1 = roll_dice (&d1);
                    result_d2 = roll_dice (&d2);    
                    uart_roll = 1;             
                    state = ROLL_DICE;
                } 
                else if (rx_data == 'p') {
                     state = OUTPUT_SCORES;                   
                }
                else if (rx_data == 'n') {
                    ser_state = AWAITING_PAYLOAD;                  
                }
            }
            break;
        
        case AWAITING_PAYLOAD:
            curr_data = rx_data;

            curr_digit = ((curr_data >= '0')  && (curr_data <= '9'))? curr_data - '0' : 10;

            if (curr_digit == 10) { //error
                ser_state = AWAITING_COMMAND;     

            } else {
                digits[digit_num] = curr_digit;

                if (digit_num == 0) {
                    digit_num = 2;
                    state = ROLL_N_TIMES;         
                    ser_state = AWAITING_COMMAND;                                  
                } else {
                    digit_num -= 1;
                }
            }
            break;

        default:
            ser_state = AWAITING_COMMAND;
    }
}//USART0_RXC_vect
