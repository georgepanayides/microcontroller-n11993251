#include "initialisation.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>   // for CCP write macro if you prefer

/* ---------------- Board pin macros: adjust if your PCB differs ---------------- */
/* 74HC595 LATCH pin (toggle to copy shift register to outputs) */
#define DISP_LATCH_PORT   PORTA
#define DISP_LATCH_PIN_bm PIN1_bm

/* We will route SPI0 to PORTC: MOSI=PC2, MISO=PC1, SCK=PC0, SS=PC3 (unused) */
#define USE_SPI0_PORTC_ALT1 1
/* ---------------------------------------------------------------------------- */

/* Optional: 20 MHz clock initialization (not used with default 3.33 MHz config) */
void clock_init_20mhz(void) {
    // Disable prescaler: PDIV=DIV1, PEN=0
    ccp_write_io((void *)&CLKCTRL.MCLKCTRLB, 0x00);
}


/* Simple GPIO init: latch as output and low */
void gpio_init(void) {
    DISP_LATCH_PORT.DIRSET = DISP_LATCH_PIN_bm;
    /* Idle latch HIGH; ISR will create a low->high (or high->low->high) pulse */
    DISP_LATCH_PORT.OUTSET = DISP_LATCH_PIN_bm;
}

/* SPI0 host for the 74HC595 (mode 0, safe baud) */
void spi_init_for_display(void) {
#if USE_SPI0_PORTC_ALT1
    /* Route SPI0 to PORTC: PC2 MOSI, PC1 MISO, PC0 SCK, PC3 SS */
    PORTMUX.SPIROUTEA = (PORTMUX.SPIROUTEA & ~0x03) | PORTMUX_SPI0_ALT1_gc;

    /* Directions: SCK, MOSI, SS = outputs; MISO = input */
    PORTC.DIRSET = PIN0_bm | PIN2_bm | PIN3_bm; /* SCK PC0, MOSI PC2, SS PC3 */
    PORTC.DIRCLR = PIN1_bm;                     /* MISO PC1 */
    PORTC.OUTSET = PIN3_bm;                     /* SS idle high (not used, but safe) */
#else
    /* Default map is on PORTA (PA0..PA3) — set DIRs similarly if you use it */
#endif

    /* SPI0: enable, host, mode 0, prescaler /4 (faster like demos), interrupt on complete */
    SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_PRESC_DIV4_gc;
    SPI0.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm; /* ignore SS input in host mode */
    SPI0.INTCTRL = SPI_IE_bm;                /* interrupt enable on transfer complete */
}

/* ---------------- Buzzer / TCA0 base setup ---------------- */
#define BUZZER_PORT     PORTB
#define BUZZER_PIN_bm   PIN5_bm     /* PB5 */

