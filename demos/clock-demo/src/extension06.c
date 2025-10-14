#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <stdint.h>

// set clock to 3.333 mhz (20 mhz / 6) so 9600 baud matches
static inline void clock_init_3p333mhz(void) {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm | CLKCTRL_PDIV_6X_gc);
}

// clock
#define F_CLK_PER     3333333UL
// uart 9600 8n1 (16x oversample)
#define UART_BAUD     9600UL
#define USART0_BAUD_VAL ((uint16_t)((F_CLK_PER * 4UL + (UART_BAUD/2)) / UART_BAUD))

// tx ring buffer
#define TX_BUFSZ 64
static volatile uint8_t tx_buf[TX_BUFSZ];
static volatile uint8_t tx_head = 0, tx_tail = 0;

typedef enum { S0, S_F, S_FO, S_FOO, S_FOOB, S_FOOBA, S_B, S_BA } state_t;
static volatile state_t sm_state = S0;

// tx helpers
static inline uint8_t tx_empty(void){ return tx_head == tx_tail; }
static inline uint8_t tx_full(void){ return (((uint8_t)(tx_head + 1)) & (TX_BUFSZ - 1)) == tx_tail; }
static inline void tx_enable_dre(void){ USART0.CTRLA |= USART_DREIE_bm; }
static inline void tx_enqueue(uint8_t c){
    if (tx_full()) return;
    tx_buf[tx_head] = c;
    tx_head = (uint8_t)(tx_head + 1) & (TX_BUFSZ - 1);
    tx_enable_dre();
}

static inline void emit_0(void){ tx_enqueue('0'); }
static inline void emit_1(void){ tx_enqueue('1'); }
static inline void emit_nl(void){ tx_enqueue('\n'); }

// usart setup (pb2 tx, pb3 rx)
static inline void uart_init(void)
{
    PORTB.DIRSET = PIN2_bm;  // tx
    PORTB.DIRCLR = PIN3_bm;  // rx
    USART0.BAUD  = USART0_BAUD_VAL;
    USART0.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc; // 8N1
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
    USART0.CTRLA = USART_RXCIE_bm;  // rx irq on
}

// dfa
static inline void sm_feed(uint8_t ch)
{
  switch (sm_state) {
    case S0:   if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; break;
    case S_F:  if (ch=='o') sm_state=S_FO; else if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; break;
    case S_FO: if (ch=='o') sm_state=S_FOO; else if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; break;
    case S_FOO:
      if (ch=='b') sm_state=S_FOOB;
      else { emit_0(); if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; }
      break;
    case S_FOOB:
      if (ch=='a') sm_state=S_FOOBA;
      else { emit_0(); if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; }
      break;
    case S_FOOBA:
      if (ch=='r') { emit_nl(); sm_state=S0; }
      else { emit_0(); if (ch=='f') sm_state=S_F; else if (ch=='b') sm_state=S_B; else sm_state=S0; }
      break;
    case S_B:  if (ch=='a') sm_state=S_BA; else if (ch=='b') sm_state=S_B; else if (ch=='f') sm_state=S_F; else sm_state=S0; break;
    case S_BA: if (ch=='r') { emit_1(); sm_state=S0; }
               else if (ch=='b') sm_state=S_B;
               else if (ch=='f') sm_state=S_F;
               else sm_state=S0; break;
  }
}

// isrs
ISR(USART0_RXC_vect) {
  uint8_t c = USART0.RXDATAL;
  sm_feed(c);
  if (!tx_empty()) tx_enable_dre();
}
ISR(USART0_DRE_vect) {
  if (tx_empty()) { USART0.CTRLA &= ~USART_DREIE_bm; }
  else { USART0.TXDATAL = tx_buf[tx_tail]; tx_tail = (uint8_t)(tx_tail + 1) & (TX_BUFSZ - 1); }
}

int main(void)
{
  clock_init_3p333mhz();  // comment out if your board already runs 3.333 MHz
  uart_init();
  sei();

  // optional: sanity banner
  // const char hello[] = "ready\n";
  // for (uint8_t i=0;i<sizeof(hello)-1;i++) tx_enqueue(hello[i]);

  for(;;) { }
}
