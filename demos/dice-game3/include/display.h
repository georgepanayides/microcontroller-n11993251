#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// 7-segment display
//     _____
//    |  A  |
//   F|     |B
//    |_____|
//    |  G  |
//   E|     |C
//    |_____|   
//       D  
                            //    ABCDEFG  xFABGCDE
#define SEGS_ZERO   0x08    // 0: 0000001  00001000
#define SEGS_ONE    0x6B    // 1: 1001111  01101011
#define SEGS_TWO    0x44    // 2: 0010010  01000100
#define SEGS_THREE  0x41    // 3: 0000110  01000001
#define SEGS_FOUR   0x23    // 4: 1001100  00100011
#define SEGS_FIVE   0x11    // 5: 0100100  00010001
#define SEGS_SIX    0x10    // 6: 0100000  00010000
#define SEGS_SEVEN  0x4B    // 7: 0001111  01001011
#define SEGS_EIGHT  0x00    // 8: 0000000  00000000
#define SEGS_NINE   0x01    // 9: 0000100  00000001
#define SEGS_A      0x02    // A: 0001000  00000010
#define SEGS_B      0x30    // B: 1100000  00110000
#define SEGS_C      0x1C    // C: 0110001  00011100
#define SEGS_D      0x60    // D: 1000010  01100000
#define SEGS_E      0x14    // E: 0110000  00010100
#define SEGS_F      0x16    // F: 0111000  00010110

#define SEGS_ALL    0x00   //     0000000  00000000
#define SEGS_BAR    0x77   //     1111110  01110111
#define SEGS_OFF    0x7F   //     1111111  01111111

void display_init(void);

void find_dec_digits(uint8_t num, uint8_t *hundreds, uint8_t *tens, uint8_t *units);
void find_hex_digits(uint8_t num, uint8_t *digit1, uint8_t *digit0);

void set_display_segments(uint8_t segs_l, uint8_t segs_r);

// Assumes num_l and num_r are in the range 0..15
void set_display_numbers(uint8_t num_l, uint8_t num_r);

void swap_display_digit(void);

#endif