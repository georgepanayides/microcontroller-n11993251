#include "buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#  define F_CPU 20000000UL
#endif

/* ----------- Pins (PA4..PA7) map to indices 0..3 ----------- */
static inline uint8_t mask_for_idx(uint8_t idx) {
    static const uint8_t m[4] = { PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };
    return (idx < 4) ? m[idx] : 0;
}

/* ----------- Debouncer state (Ganssle 2-bit vertical counter) ----------- */
static volatile uint8_t debounced_state = (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm); /* 1=not pressed (pull-up) */
static uint8_t ct0, ct1;

/* ----------- Tiny FIFO for press events (falling edges) ----------- */
#define QSZ 8u
static volatile uint8_t q_buf[QSZ];
static volatile uint8_t q_head = 0, q_tail = 0;

static inline uint8_t q_next(uint8_t x) { return (uint8_t)((x + 1) & (QSZ - 1)); }

void buttons_enqueue(uint8_t idx) {
    if (idx > 3) return;
    uint8_t nh = q_next(q_head);
    if (nh != q_tail) {            /* drop if full */
        q_buf[q_head] = idx;
        q_head = nh;
    }
}

void buttons_inject(uint8_t idx) {
    buttons_enqueue(idx);
}

int8_t buttons_pop(void) {
    if (q_head == q_tail) return -1;
    uint8_t v = q_buf[q_tail];
    q_tail = q_next(q_tail);
    return (int8_t)v;
}

void buttons_clear_all(void) {
    q_tail = q_head;
}

/* True while physically down (active-low) */
uint8_t buttons_is_down(uint8_t idx) {
    uint8_t m = mask_for_idx(idx);
    return (m && ((debounced_state & m) == 0)) ? 1u : 0u;
}

/* ----------- Init: PA4..PA7 pull-ups, TCB1 periodic @ 5 ms ----------- */
void buttons_init(void)
{
    /* Enable pull-ups; leave ISC to default (we poll via TCB1) */
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

    /* Prime debounced state with current input */
    debounced_state = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

    /* TCB1: periodic interrupt every 5 ms
       Use CLK_PER/2 -> 10 MHz at 20 MHz F_CPU
       CCMP = 10,000,000 * 0.005 - 1 = 49,999
    */
    TCB1.CTRLA   = 0;                       /* disable while configuring */
    TCB1.CNT     = 0;
    TCB1.CCMP    = 49999;
    TCB1.CTRLB   = TCB_CNTMODE_INT_gc;      /* Periodic interrupt mode */
    TCB1.INTFLAGS = TCB_CAPT_bm;            /* clear */
    TCB1.INTCTRL  = TCB_CAPT_bm;            /* enable IRQ */
    TCB1.CTRLA   = TCB_ENABLE_bm | TCB_CLKSEL_DIV2_gc;
}

/* ----------- 5 ms debouncer ISR: detect falling edges -> enqueue ----------- */
ISR(TCB1_INT_vect)
{
    /* Read raw pins, keep only PA4..PA7 */
    uint8_t sample = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

    uint8_t delta = sample ^ debounced_state;      /* bits that changed since last stable */
    ct0 = ~(ct0 & delta);
    ct1 =  (ct1 ^ ct0) & delta;
    uint8_t toggle = delta & ct0 & ct1;

    if (toggle) {
        debounced_state ^= toggle;                 /* commit new stable bits */

        /* Falling edges are those that toggled and are now 0 (pressed) */
        uint8_t falling = toggle & ~debounced_state;

        if (falling & PIN4_bm) buttons_enqueue(0); /* S1 */
        if (falling & PIN5_bm) buttons_enqueue(1); /* S2 */
        if (falling & PIN6_bm) buttons_enqueue(2); /* S3 */
        if (falling & PIN7_bm) buttons_enqueue(3); /* S4 */
    }

    TCB1.INTFLAGS = TCB_CAPT_bm;                   /* ack */
}
