#include "buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

#ifndef F_CPU
#  define F_CPU 3333333UL
#endif

/* Logical indices 0..3 map to S1..S4 on PA4..PA7 */
static inline uint8_t mask_for_idx(uint8_t idx) {
    static const uint8_t m[4] = { PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };
    return (idx < 4) ? m[idx] : 0;
}

/* Debouncer state */
static volatile uint8_t debounced_state = (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);
static uint8_t diff_count[4];
static uint8_t pending_press[4];
static uint32_t pending_t0[4];

/* FIFO for press events */
#define QSZ 8u
static volatile uint8_t q_buf[QSZ];
static volatile uint8_t q_head = 0, q_tail = 0;

static inline uint8_t q_next(uint8_t x) { return (uint8_t)((x + 1) & (QSZ - 1)); }

void buttons_enqueue(uint8_t idx) {
    if (idx > 3) return;
    uint8_t nh = q_next(q_head);
    if (nh != q_tail) {
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

/* True while physically down */
uint8_t buttons_is_down(uint8_t idx) {
    uint8_t m = mask_for_idx(idx);
    return (m && ((debounced_state & m) == 0)) ? 1u : 0u;
}

/* Init: PA4..PA7 pull-ups, TCB1 periodic @ 5 ms */
void buttons_init(void)
{
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

    debounced_state = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

     /* TCB1: 5ms periodic */
     TCB1.CTRLA   = 0;
     TCB1.CNT     = 0;
     TCB1.CCMP    = 16667;
     TCB1.CTRLB   = TCB_CNTMODE_INT_gc;
     TCB1.INTFLAGS = TCB_CAPT_bm;
     TCB1.INTCTRL  = TCB_CAPT_bm;
     TCB1.CTRLA   = TCB_ENABLE_bm;
}

/* 5 ms debouncer ISR */
ISR(TCB1_INT_vect)
{
    uint8_t sample = PORTA.IN & (PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm);

    /* For each button, debounce based on 3 consecutive samples */
    for (uint8_t idx = 0; idx < 4; idx++) {
        uint8_t mask = mask_for_idx(idx);
        uint8_t raw_bit = (sample & mask);
        uint8_t deb_bit = (debounced_state & mask);

        if ((raw_bit != 0) != (deb_bit != 0)) {
            if (raw_bit == 0) {
                if (!pending_press[idx]) {
                    pending_press[idx] = 1;
                    pending_t0[idx] = millis();
                }
                if (diff_count[idx] < 3) diff_count[idx]++;
                if (diff_count[idx] >= 3) {
                    debounced_state &= (uint8_t)~mask;
                    buttons_enqueue(idx);
                    diff_count[idx] = 0;
                    pending_press[idx] = 0;
                }
            } else {
                if (deb_bit != 0) {
                    if (pending_press[idx]) {
                        uint32_t dt = millis() - pending_t0[idx];
                        if (dt >= 12u) {
                            buttons_enqueue(idx);
                        }
                        pending_press[idx] = 0;
                    }
                    diff_count[idx] = 0;
                } else {
                    if (diff_count[idx] < 3) diff_count[idx]++;
                    if (diff_count[idx] >= 3) {
                        debounced_state |= mask;
                        diff_count[idx] = 0;
                    }
                }
            }
        } else {
            diff_count[idx] = 0;
            if (raw_bit != 0) pending_press[idx] = 0;
        }
    }

    /* Multiplex display every 5 ms */
    extern void display_multiplex(void);
    display_multiplex();

    TCB1.INTFLAGS = TCB_CAPT_bm;
}
