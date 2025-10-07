#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#  define F_CPU 20000000UL
#endif

static volatile uint32_t g_ms = 0;

void tcb0_init_1ms(void) {
    /* Periodic interrupt every 1 ms:
       CCMP = F_CPU/1000 - 1 when CLK_PER (DIV1): 20000000/1000 - 1 = 19999
    */
    TCB0.CCMP   = (uint16_t)(F_CPU/1000UL - 1UL);
    TCB0.CTRLB  = TCB_CNTMODE_INT_gc;                 /* Periodic Interrupt mode */
    TCB0.INTCTRL = TCB_CAPT_bm;                       /* enable interrupt */
    TCB0.CTRLA  = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm; /* CLK_PER, enable */
}

ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm; /* clear */
    g_ms++;
}

uint32_t millis(void) {
    uint32_t m;
    uint8_t s = SREG; cli(); m = g_ms; SREG = s;
    return m;
}

void delay_ms(uint16_t ms) {
    uint32_t start = millis();
    while ((uint32_t)(millis() - start) < ms) { /* spin */ }
}
