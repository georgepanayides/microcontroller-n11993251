/**
 * @file qutyserial.h
 * @brief Serial communication library for the ATtiny1626.
 *
 * @details This library provides functions to initialise serial
 * communication and standard IO streams for the ATtiny1626.
 *
 * The following ISRs have been defined in this library:
 * - USART0 Receive Complete (RXC)
 */

#include <stdint.h>

/**
 * Initialises the USART0 peripheral and redirects the stdin and
 * stdout streams.
 *
 * Communication is configured to 9600 baud 8N1 at 3.333 MHz.
 *
 * @warning - PB2 must be output enabled.
 * @warning - PB3 must be input enabled.
 * @warning - Interrupts must be enabled globally.
 */
void serial_init(void);

/**
 * Returns the number of bytes in the receive buffer.
 *
 * @return The number of bytes in the receive buffer.
 */
uint8_t serial_bytes_available(void);
