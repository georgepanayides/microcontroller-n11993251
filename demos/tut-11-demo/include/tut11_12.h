// tut11_12.h

/**
 * @file tut11_12.h
 * @brief Functions used in Tutorial 11 and Tutorial 12.
 */

#include <stdint.h>

// 32-bit Linear Feedback Shift Register ---------------------------- //

/**
 * Set the current seed of the random number generator.
 *
 * @param seed The new seed.
 */
void set_seed(uint32_t seed);

/**
 * Get the current seed of the random number generator.
 *
 * @return The current seed.
 */
uint32_t get_seed(void);

/**
 * Generate a 32-bit pseudo-random number.
 *
 * @return A 32-bit number.
 */
uint32_t rng(void);

// Initialisation --------------------------------------------------- //

/**
 * Pull-up enables pins PA4-PA7.
 */
void button_init(void);

/**
 * Configures PWM through TCA0 and necessary pins.
 *
 * @note - PER is set to 1 and CMP0 is set to 0.
 * @note - Prescaler is set to 2.
 * @warning - PB0 must be output enabled.
 */
void pwm_init(void);

/**
 * Configures SPI for display multiplexing and necessary pins.
 *
 * @warning - PA1 must be output enabled.
 * @warning - PC0 and PC2 must be output enabled.
 * @warning - Interrupts must be enabled globally.
 */
void spi_init(void);

/**
 * Configures TCB0 for timing events and configures TCB1 for pushbutton
 * debouncing and display multiplexing. Also configures necessary pins.
 *
 * @note - TCB0 generates interrupts every 1 ms.
 * @note - TCB1 generates interrupts every 10 ms.
 * @warning - Interrupts must be enabled globally.
 */
void timer_init(void);

// Timers  ---------------------------------------------------------- //

// A count of elapsed time in milliseconds
extern volatile uint16_t elapsed_time;

// A debounced sample of the pushbuttons
extern volatile uint8_t pb_debounced_state;

/**
 * Update the bytes transmitted to the 7-segment display.
 *
 * @param left The left display value.
 * @param right The right display value.
 *
 * @note - Ensure bit 7 is cleared for both parameters.
 */
void update_display(const uint8_t left, const uint8_t right);
