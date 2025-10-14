#include "buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"  /* for millis() timestamping */

#ifndef F_CPU
#  define F_CPU 3333333UL  /* Default QUTy platform clock */
#endif

/* ----------- Logical indices 0..3 map to S1..S4 on PA4..PA7 ----------- */
/* QUTy hardware: S1=PA4, S2=PA5, S3=PA6, S4=PA7 (left to right) */
static inline uint8_t mask_for_idx(uint8_t idx) {
    static const uint8_t m[4] = { PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };
    return (idx < 4) ? m[idx] : 0;
}

/* ----------- Debouncer state (require 3 consecutive samples ≈ 15 ms) ----------- */
static volatile uint8_t debounced_state = (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm); /* 1=not pressed (pull-up) */
/* Per-button counters for consecutive samples differing from debounced_state */
static uint8_t diff_count[4]; /* indices correspond to S1..S4 */
/* Track candidate press timing to robustly accept 15 ms taps regardless of sampling phase */
static uint8_t pending_press[4];
static uint32_t pending_t0[4];

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

     /* TCB1: periodic interrupt every 5 ms using CLK_PER (no divider)
         Demo-equivalent timing: CCMP = 3,333,333 * 0.005 ≈ 16667
     */
     TCB1.CTRLA   = 0;                       /* disable while configuring */
     TCB1.CNT     = 0;
     TCB1.CCMP    = 16667;                   /* 5ms at nominal 3.333 MHz */
     TCB1.CTRLB   = TCB_CNTMODE_INT_gc;      /* Periodic interrupt mode */
     TCB1.INTFLAGS = TCB_CAPT_bm;            /* clear */
     TCB1.INTCTRL  = TCB_CAPT_bm;            /* enable IRQ */
     TCB1.CTRLA   = TCB_ENABLE_bm;           /* CLK_PER, enable (no DIV) */
}

/* ----------- 5 ms debouncer ISR: detect falling edges -> enqueue ----------- */
ISR(TCB1_INT_vect)
{
    /* Read raw pins, keep only PA4..PA7 */
    uint8_t sample = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

    /* For each button, if the raw sample differs from debounced, increment its
       diff counter; else reset. When counter reaches 3 (≈15 ms), accept change. */
    for (uint8_t idx = 0; idx < 4; idx++) {
        uint8_t mask = mask_for_idx(idx);
        uint8_t raw_bit = (sample & mask);
        uint8_t deb_bit = (debounced_state & mask);

        if ((raw_bit != 0) != (deb_bit != 0)) {
            /* differs from debounced */
            if (raw_bit == 0) {
                /* Candidate press (active-low) */
                if (!pending_press[idx]) {
                    pending_press[idx] = 1;
                    pending_t0[idx] = millis();
                }
                if (diff_count[idx] < 3) diff_count[idx]++;
                if (diff_count[idx] >= 3) {
                    /* Commit new stable press */
                    debounced_state &= (uint8_t)~mask; /* pressed (0) */
                    buttons_enqueue(idx); /* enqueue falling edge */
                    diff_count[idx] = 0;
                    pending_press[idx] = 0; /* consumed */
                }
            } else {
                /* raw returned high (release) while differing from debounced */
                if (deb_bit != 0) {
                    /* We were not committed pressed; treat as short tap candidate */
                    if (pending_press[idx]) {
                        uint32_t dt = millis() - pending_t0[idx];
                        if (dt >= 12u) {
                            /* Accept as a valid quick tap despite not hitting 3 samples */
                            buttons_enqueue(idx);
                        }
                        pending_press[idx] = 0;
                    }
                    diff_count[idx] = 0; /* cancel pending change */
                    /* debounced stays released */
                } else {
                    /* We were committed pressed (deb_bit==0), now transitioning to release */
                    if (diff_count[idx] < 3) diff_count[idx]++;
                    if (diff_count[idx] >= 3) {
                        debounced_state |= mask;  /* commit release (1) */
                        diff_count[idx] = 0;
                        /* no enqueue on release */
                    }
                }
            }
        } else {
            /* same as debounced: reset counter */
            diff_count[idx] = 0;
            if (raw_bit != 0) pending_press[idx] = 0; /* clear pending if line is high */
        }
    }

    /* Also multiplex the 7-seg every 5 ms (matches demos) */
    extern void display_multiplex(void);
    display_multiplex();

    TCB1.INTFLAGS = TCB_CAPT_bm;                   /* ack */
}
