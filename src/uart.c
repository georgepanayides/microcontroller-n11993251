#include "uart.h"
#include <avr/io.h>
#include "buzzer.h"     /* to retune live tone */
#include "buttons.h"    /* buttons_inject() */
#include "sequencing.h" /* extern const uint16_t step_freq[4] */

/* ============ Low-level USART0 @ 20MHz, 9600-8N1 ============ */
void uart_init_9600_8n1(void)
{
    /* PB2=TXD out, PB3=RXD in */
    PORTB.DIRSET = PIN2_bm;
    PORTB.DIRCLR = PIN3_bm;

    /* Default route (PB2/PB3) */
    /* PORTMUX.USARTROUTEA default 0 -> USART0 on PORTB */

    /* 9600 baud @ 20 MHz, normal 16x oversampling:
       BAUD = (F_PER * 64) / (16 * baud)
       -> (20,000,000 * 64) / (16 * 9600) = 8,333.33 -> 8333
    */
    USART0.BAUD = 8333;

    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
                 | USART_PMODE_DISABLED_gc
                 | USART_SBMODE_1BIT_gc
                 | USART_CHSIZE_8BIT_gc;    /* 8N1 */

    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; /* enable Rx/Tx */
}

int uart_getc_nonblock(void)
{
    if (USART0.STATUS & USART_RXCIF_bm) {
        return (int)USART0.RXDATAL;
    }
    return -1;
}

void uart_putc(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm)) { }
    USART0.TXDATAL = c;
}

void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

void uart_put_u16_dec(uint16_t v)
{
    char buf[6];   /* max "65535" + NUL */
    char *p = &buf[5];
    *p = '\0';
    do {
        *--p = (char)('0' + (v % 10));
        v /= 10;
    } while (v);
    uart_puts(p);
}

/* ================= Game command layer ================= */

/* live/global command state */
static volatile uint8_t  g_cmd_reset = 0;
static volatile uint8_t  g_seed_pending = 0;
static volatile uint32_t g_seed_value = 0;
static volatile int16_t  g_freq_offset = 0;   /* retained until RESET */

uint16_t uart_apply_freq_offset(uint16_t base_hz)
{
    int32_t f = (int32_t)base_hz + g_freq_offset;
    if (f < 1) f = 1;
    return (uint16_t)f;
}

uint8_t uart_take_reset_and_clear(void)
{
    uint8_t r = g_cmd_reset; g_cmd_reset = 0; return r;
}

uint8_t uart_seed_pending(void) { return g_seed_pending; }
uint32_t uart_take_seed(void) { g_seed_pending = 0; return g_seed_value; }

/* small helpers */
static inline uint8_t is_hex_lower(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }
static inline uint8_t hexval(char c) { return (c <= '9') ? (uint8_t)(c - '0') : (uint8_t)(10 + c - 'a'); }
static inline uint8_t is_s_key(int c, uint8_t *out_idx)
{
    switch (c) {
        case '1': case 'q': *out_idx = 0; return 1; /* S1 */
        case '2': case 'w': *out_idx = 1; return 1; /* S2 */
        case '3': case 'e': *out_idx = 2; return 1; /* S3 */
        case '4': case 'r': *out_idx = 3; return 1; /* S4 */
        default: return 0;
    }
}
static inline uint8_t is_inc(int c) { return (c == ',' || c == 'k'); }
static inline uint8_t is_dec(int c) { return (c == '.' || c == 'l'); }
static inline uint8_t is_reset(int c){ return (c == '0' || c == 'p'); }
static inline uint8_t is_seed(int c) { return (c == '9' || c == 'o'); }

/* While buzzer is active, INC/DEC retunes immediately by ±1 Hz (spec: immediate). */
static inline void nudge_live_frequency(uint16_t *active_hz, int8_t delta)
{
    if (*active_hz == 0) return;               /* no tone right now */
    int32_t f = (int32_t)(*active_hz) + delta;
    if (f < 1) f = 1;
    *active_hz = (uint16_t)f;
    buzzer_start_hz(*active_hz);               /* retune now */
}

/* Poll and interpret commands.
   game_state is your enum value (cast to uint8_t), used only to decide
   whether to inject S-keys (we inject always; the game will accept them
   only in GS_INPUT).
   last_base_hz is the “unmodified” step frequency if a tone is currently
   playing, or 0 if silent. We keep active_hz as a mutable live value to
   support INC/DEC in-place. */
void uart_poll_commands(uint8_t game_state, uint16_t last_base_hz, uint16_t *active_hz)
{
    (void)game_state;
    int c;
    static uint8_t seed_collecting = 0;
    static uint8_t seed_count = 0;
    static uint32_t seed_acc = 0;

    while ((c = uart_getc_nonblock()) >= 0) {

        if (seed_collecting) {
            if (is_hex_lower((char)c)) {
                seed_acc = (seed_acc << 4) | hexval((char)c);
                if (++seed_count == 8) {
                    g_seed_value = seed_acc;
                    g_seed_pending = 1;
                    seed_collecting = 0;
                }
            } else {
                /* invalid -> cancel without changing pending seed */
                seed_collecting = 0;
            }
            continue;
        }

        /* normal key handling */
        uint8_t idx;
        if (is_s_key(c, &idx)) {
            buttons_inject(idx);               /* inject into same FIFO as buttons */
        } else if (is_inc(c)) {
            if (*active_hz == 0 && last_base_hz != 0) *active_hz = uart_apply_freq_offset(last_base_hz);
            nudge_live_frequency(active_hz, +1);
            g_freq_offset += 1;                /* persist */
        } else if (is_dec(c)) {
            if (*active_hz == 0 && last_base_hz != 0) *active_hz = uart_apply_freq_offset(last_base_hz);
            nudge_live_frequency(active_hz, -1);
            g_freq_offset -= 1;                /* persist */
        } else if (is_reset(c)) {
            g_cmd_reset = 1;
            g_freq_offset = 0;
        } else if (is_seed(c)) {
            seed_collecting = 1;
            seed_count = 0;
            seed_acc = 0;
        } else {
            /* ignore anything else per spec */
        }
    }
}

/* Print during patterns (call while you multiplex the digits) */
void uart_game_success(uint16_t score)
{
    uart_puts("SUCCESS\n");
    uart_put_u16_dec(score);
    uart_putc('\n');
}

void uart_game_over(uint16_t score)
{
    uart_puts("GAME OVER\n");
    uart_put_u16_dec(score);
    uart_putc('\n');
}
