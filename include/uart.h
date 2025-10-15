// #ifndef UART_H
// #define UART_H

// #include <stdint.h>

// /* Basic UART functions - like studio12 demo */
// void uart_init_9600_8n1(void);          /* PB2=TXD, simple setup */
// void uart_putc(uint8_t c);              /* Send single character */

// /* Simon Says specific functions */
// uint16_t uart_apply_freq_offset(uint16_t base_hz);  /* Pass-through for now */
// uint8_t uart_seed_pending(void);                   /* Always returns 0 */  
// uint32_t uart_take_seed(void);                     /* Always returns 0 */
// void uart_game_success(uint16_t score);            /* Print success message */
// void uart_game_over(uint16_t score);               /* Print game over message */

// #endif
