#ifndef UART_H
#define UART_H

#include <stdint.h>

/* ----- Low-level driver ----- */
void uart_init_9600_8n1(void);          /* PB2=TXD, PB3=RXD, 20MHz F_CPU */
int  uart_getc_nonblock(void);          /* returns -1 if no byte */
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_u16_dec(uint16_t v);      /* decimal ASCII, no leading zeros */
/* Apply current persistent frequency offset (retained until RESET). */
uint16_t uart_apply_freq_offset(uint16_t base_hz);

/* ----- Game command layer ----- */
/* You call this often (each loop + inside any 1ms polling windows) */
void uart_poll_commands(uint8_t game_state,
                        uint16_t last_base_hz, /* 0 if silence */
                        uint16_t *active_hz);  /* in/out: live freq if buzzer ON */

/* Called when patterns are showing (during those 1ms multiplex loops) */
void uart_game_success(uint16_t score);
void uart_game_over(uint16_t score);

/* Hooks the game checks at round boundaries */
uint8_t uart_take_reset_and_clear(void);       /* 1 => reset requested */
uint8_t uart_seed_pending(void);
uint32_t uart_take_seed(void);                  /* returns and clears pending */

/* Expose key->button injector for the game (you already have buttons_pop).
   Implemented in buttons.c as a 1-byte enqueue into the same FIFO. */
void buttons_inject(uint8_t idx);   /* 0..3 for S1..S4 */

#endif
