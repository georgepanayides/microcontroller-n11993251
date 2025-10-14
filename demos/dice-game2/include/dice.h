#include <stdint.h>

typedef struct dice {
    uint32_t LFSR;
} Dice;

void init_dice(Dice* d, uint32_t lfsr);
uint8_t roll_dice(Dice* d);