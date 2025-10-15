#include "highscore.h"
#include <avr/io.h>
#include <stdint.h>
#include "timer.h"
#include "uart.h"

#ifndef F_CPU
#  define F_CPU 20000000UL
#endif

/* UART helpers */
static inline uint8_t hs_uart_rx_ready(void) {
    return (USART0.STATUS & USART_RXCIF_bm) ? 1u : 0u;
}
static inline char hs_uart_getc_now(void) {
    return USART0.RXDATAL;
}
static inline void hs_uart_putc(char c) {
    while (!(USART0.STATUS & USART_DREIF_bm)) { /* wait */ }
    USART0.TXDATAL = c;
}
static inline void hs_uart_puts(const char *s) {
    while (*s) hs_uart_putc(*s++);
}

/* Table in SRAM */
#define HS_MAX 5
typedef struct { char name[21]; uint16_t score; uint8_t used; } hs_entry_t;
static hs_entry_t hs[HS_MAX];

/* Utilities */
static void hs_print_u16(uint16_t v) {
    char buf[6];
    int8_t i = 0;
    if (v == 0) { hs_uart_putc('0'); return; }
    while (v > 0 && i < 5) { buf[i++] = '0' + (v % 10u); v /= 10u; }
    while (--i >= 0) hs_uart_putc(buf[i]);
}

static uint8_t hs_size(void) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < HS_MAX; i++) if (hs[i].used) n++;
    return n;
}

/* Find insertion index for score (descending) */
static uint8_t hs_find_pos(uint16_t score) {
    uint8_t n = hs_size();
    for (uint8_t i = 0; i < n; i++) {
        if (!hs[i].used || score > hs[i].score) return i;
    }
    return n; /* at end */
}

/* Insert (name,score) at pos, shifting down and trimming to HS_MAX */
static void hs_insert_at(uint8_t pos, const char *name, uint16_t score) {
    uint8_t n = hs_size();
    if (n < HS_MAX) n++; /* room grows by one */
    if (pos > n) pos = n;

    /* shift down */
    for (int8_t i = (int8_t)(n - 1); i > (int8_t)pos; i--) {
        hs[i] = hs[i - 1];
    }

    /* copy new */
    hs[pos].score = score;
    hs[pos].used  = 1;

    /* copy up to 20 chars, NUL-terminate */
    uint8_t k = 0;
    while (name[k] && k < 20) { hs[pos].name[k] = name[k]; k++; }
    hs[pos].name[k] = '\0';
}

uint8_t highscore_qualifies(uint16_t score) {
    uint8_t n = hs_size();
    if (n < HS_MAX) return 1;
    // table full
    return (score > hs[n - 1].score) ? 1u : 0u;
}

void highscore_prompt_and_store(uint16_t score) {
    extern volatile uint16_t elapsed_time;
    char name[21];
    uint8_t len = 0;

    hs_uart_puts("Enter name: ");

    const uint16_t TOUT_MS = 5000;
    elapsed_time = 0;
    uint16_t last_char_time = 0;

    while (1) {
        if (hs_uart_rx_ready()) {
            char c = hs_uart_getc_now();

            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                break;
            }

            if (len < 20) {
                name[len++] = c;
            }
            last_char_time = elapsed_time;
        }

        if (len == 0) {
            if (elapsed_time >= TOUT_MS) break;
        } else {
            if ((elapsed_time - last_char_time) >= TOUT_MS) break;
        }
    }

    name[len] = '\0';

    /* store into table (descending) */
    uint8_t pos = hs_find_pos(score);
    if (pos < HS_MAX) {
        hs_insert_at(pos, name, score);
    }

    /* then print table */
    hs_uart_puts("\n");
    for (uint8_t i = 0; i < HS_MAX; i++) {
        if (!hs[i].used) break;
        const char *p = hs[i].name;
        while (*p) hs_uart_putc(*p++);
        hs_uart_putc(' ');
        hs_print_u16(hs[i].score);
        hs_uart_putc('\n');
    }
}

void highscore_print_table(void) {
    for (uint8_t i = 0; i < HS_MAX; i++) {
        if (!hs[i].used) break;
        const char *p = hs[i].name;
        while (*p) hs_uart_putc(*p++);
        hs_uart_putc(' ');
        hs_print_u16(hs[i].score);
        hs_uart_putc('\n');
    }
}
