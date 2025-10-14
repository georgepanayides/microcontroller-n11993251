/**
 * @file qutyio.h
 * @brief I/O library for the QUTy microcontroller.
 *
 * @details This library provides functions to initialise serial
 * communication via SPI and USART to control the 7-segment display and
 * provide standard IO streams for the ATtiny1626.
 *
 * The following ISRs have been defined in this library:
 * - TCB1 Capture (CAPT)
 * - SPI0 (SPI)
 * - USART0 Receive Complete (RXC)
 */

#include <stdint.h>

/**
 * Initialises the TCB1 and SPI0 peripherals, and turns the display OFF
 * initially.
 *
 * The display refresh rate is set to 50 Hz at 3.333 MHz.
 *
 * @warning - PA1, PB1, PC0, PC2 must be output enabled.
 * @warning - PC3 must be input enabled.
 * @warning - Interrupts must be enabled globally.
 *
 * @note This function disables and re-enables interrupts globally.
 */
void display_init(void);

/**
 * Turns the display ON.
 */
void display_on(void);

/**
 * Turns the display OFF.
 */
void display_off(void);

/**
 * Displays an unsigned 8-bit integer on the 7-segment display in
 * hexadecimal.
 *
 * @param value The unsigned 8-bit integer to display.
 */
void display_hex(uint8_t value);

/**
 * Sets the segment states for the left and right digits of the
 * 7-segment display according to the mapping on the QUTy
 * microcontroller.
 *
 * @param left The segment states for the left digit.
 * @param right The segment states for the right digit.
 */
void display_raw(uint8_t left, uint8_t right);

/**
 * Sets the display brightness.
 *
 * @param value The brightness value.
 */
void display_brightness(uint8_t value);

/**
 * Initialises the USART0 peripheral and redirects the stdin and
 * stdout streams.
 *
 * Communication is configured to 9600 baud 8N1 at 3.333 MHz.
 *
 * @warning - PB2 must be output enabled.
 * @warning - PB3 must be input enabled.
 * @warning - Interrupts must be enabled globally.
 *
 * @note This function disables and re-enables interrupts globally.
 */
void serial_init(void);

/**
 * Returns the number of bytes in the receive buffer.
 *
 * @return The number of bytes in the receive buffer.
 */
uint8_t serial_bytes_available(void);
