#include <stdint.h>
#include <stdio.h>

#include "dice.h"

//uint32_t LFSR = 0xCAB202;

const uint32_t MASK = 0xE10000;

void init_dice(Dice* d, uint32_t lfsr) 
{
    d->LFSR = lfsr;
}//init_dice

void lfsr_next(Dice* d)
{
    uint8_t lsb = d->LFSR & 0b1;
    d->LFSR >>= 1;

    if (lsb) 
    {
        d->LFSR ^= MASK;
    }
}

uint8_t roll_dice(Dice* d)
{
    uint8_t result = 0;

    do
    {
        lfsr_next(d);
        result = d->LFSR & 0b111; // Between 0-7
    } while (result > 5); // Rejection sampling

    return result + 1;
}