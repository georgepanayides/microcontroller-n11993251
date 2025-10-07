#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdint.h>

/* Returns 1 if score would place in the top 5, 0 otherwise. */
uint8_t highscore_qualifies(uint16_t score);

/* Blocks while prompting & collecting name (max 20 chars) with 5 s rules.
   Inserts into table if it still qualifies (it will). */
void highscore_prompt_and_store(uint16_t score);

/* Prints each entry as "<name> <score>\n" in descending score order. */
void highscore_print_table(void);

#endif
