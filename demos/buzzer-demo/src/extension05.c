#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "qutyserial.h"

/** EX: E5.0

TASK:

Your task is to implement a two-tone siren using the QUTy. You must use
one of the Waveform Outputs of TCA0 to drive the buzzer with a 50 % duty
cycle signal.

Your siren must alternate between the following frequencies:

    f1 = 2AB0 Hz with f1 active for t1 = 3E0 ms
    f2 = 4CD0 Hz with f2 active for t2 = 6F0 ms

where A-F are the 2nd through 7th digits of your student number:

    n*ABCDEF*

EXAMPLE: If your student number was n12345678, then

A = 2, B = 3, C = 4, D = 5, E = 6, F = 7

f1 = 2230 Hz
f2 = 4450 Hz
t1 = 360 ms
t2 = 670 ms

NOTES:

Your program must use either TCA or TCB to time the duration of each
tone. Do not use software based delays.

Your program must be interrupt driven. The main() function expects
init() to return to demonstrate this.

You may define additional variables and functions that are used by
init(), in this file.
*/

// my student number n11993251

// values
#define F1_HZ   2190u
#define F2_HZ   4930u
#define T1_MS    320u
#define T2_MS    650u

// clock
#define F_CLK_PER   3333333UL   

// tcb tick
#define TCB_TICK_MS  1u

// buzzer pin
#define BUZZER_DIR_REG  PORTB.DIR
#define BUZZER_DIRSET   PIN0_bm  // pb0 out

// state
static volatile uint8_t  s_tone_idx = 0; 
static volatile uint16_t s_ticks_left = 0;  

// make per/cmp for 50% duty
static inline void tca_calc_50pct(uint16_t hz, uint16_t *per_out, uint16_t *cmp_out)
{
    uint32_t per = (F_CLK_PER + (hz / 2u)) / (uint32_t)hz; // round
    if (per == 0) per = 1;
    per -= 1;
    uint16_t per16 = (uint16_t)per;
    uint16_t cmp16 = (uint16_t)((per16 + 1u) / 2u);
    *per_out = per16;
    *cmp_out = cmp16;
}

// set freq now
static inline void tca0_set_freq_wo0_immediate(uint16_t hz)
{
    uint16_t per, cmp;
    tca_calc_50pct(hz, &per, &cmp);
    TCA0.SINGLE.PER  = per;
    TCA0.SINGLE.CMP0 = cmp;
}

// queue freq change
static inline void tca0_queue_freq_wo0(uint16_t hz)
{
    uint16_t per, cmp;
    tca_calc_50pct(hz, &per, &cmp);
    TCA0.SINGLE.CMP0BUF = cmp;
    TCA0.SINGLE.PERBUF  = per;
}

// setup tca0 pwm on pb0 (wo0)
static inline void tca0_init_pwm_pb0(void)
{
    PORTMUX.TCAROUTEA = 0;

    BUZZER_DIR_REG |= BUZZER_DIRSET; // pb0 out

    TCA0.SINGLE.CTRLA = 0;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc
                      | TCA_SINGLE_CMP0EN_bm;

    tca0_set_freq_wo0_immediate(F1_HZ); // start on f1 cleanly

    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm
                      | TCA_SINGLE_CLKSEL_DIV1_gc; 
}

// setup tcb0 1 ms periodic interrupt
static inline void tcb0_init_tick_1ms(void)
{
    const uint32_t tcb_clk = F_CLK_PER / 2u;
    const uint16_t ccmp = (uint16_t)(((tcb_clk * TCB_TICK_MS) + 500u) / 1000u) - 1u;

    TCB0.CTRLA = 0;// off while config
    TCB0.CNT   = 0;
    TCB0.CCMP  = ccmp;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; 
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear flag
    TCB0.INTCTRL  = TCB_CAPT_bm;  
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_DIV2_gc; 
}

// how many ticks for this tone
static inline uint16_t duration_ticks_for(uint8_t tone_idx)
{
    return (tone_idx == 0) ? T1_MS : T2_MS; 
}

// 1 ms isr
ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear

    if (s_ticks_left) {
        s_ticks_left--;
        return;
    }

    // flip tone
    s_tone_idx ^= 1u;
    if (s_tone_idx == 0) tca0_queue_freq_wo0(F1_HZ);
    else                 tca0_queue_freq_wo0(F2_HZ);

    // reload time
    s_ticks_left = duration_ticks_for(s_tone_idx);
}

// This function is called once by the main function.

void init(void)
{
    tca0_init_pwm_pb0(); // start pwm
    s_tone_idx = 0;
    s_ticks_left = duration_ticks_for(0);
    tcb0_init_tick_1ms();
    sei();                        
}

/** CODE: Write your code for Ex E5.0 above this line. */