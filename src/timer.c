#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#  define F_CPU 3333333UL  /* Default QUTy platform clock - NOTE: actual may vary! */
#endif

static volatile uint32_t g_ms = 0;

void tcb0_init_1ms(void) {
    /* Periodic interrupt every 1 ms:
       Using hardcoded CCMP value matching clock-demo-2 approach
       clock-demo-2 uses CCMP=52083 for 1/64 second (15.625ms)
       For 1ms: 52083 / 15.625 = 3333.312, round to 3333
    */
    TCB0.CCMP   = 3333;  /* Hardcoded for 1ms at nominal 3.333MHz */
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
