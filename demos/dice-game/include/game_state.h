#ifndef GAMESTATE_H
#define GAMESTATE_H

typedef enum {
    AWAITING_INPUT,
    ROLL_DICE,
    DISPLAY_DICE,
    DISPLAY_SCORE,
    DISPLAY_PATTERN,    
    OUTPUT_SCORES,
    ROLL_N_TIMES
} Game_State;

#endif