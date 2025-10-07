#ifndef INITIALISATION_H
#define INITIALISATION_H

#include <stdint.h>

/* 20 MHz, no prescaler */
void clock_init_20mhz(void);
void gpio_init(void);
void spi_init_for_display(void);

#endif