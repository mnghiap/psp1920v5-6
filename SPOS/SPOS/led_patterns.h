/*! \file 
 *  \brief CUSTOM CHAR PATTERNS
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2019
 */

#define LED_CHAR_HEIGHT_SMALL 5
#define LED_CHAR_WIDTH_SMALL 3
#define LED_CHAR_HEIGHT_LARGE 8
#define LED_CHAR_WIDTH_LARGE 5

//! Defines a custom CHAR out of eight rows passed as integer values
#define LED_CUSTOM_CHAR(cc0,cc1,cc2,cc3,cc4,cc5,cc6,cc7) 0 \
| (((uint64_t)(cc0)) << 0*8) \
| (((uint64_t)(cc1)) << 1*8) \
| (((uint64_t)(cc2)) << 2*8) \
| (((uint64_t)(cc3)) << 3*8) \
| (((uint64_t)(cc4)) << 4*8) \
| (((uint64_t)(cc5)) << 5*8) \
| (((uint64_t)(cc6)) << 6*8) \
| (((uint64_t)(cc7)) << 7*8)

#define MATRIX_1 (LED_CUSTOM_CHAR( \
0b11111111, \
0b10000001, \
0b10000001, \
0b10000001, \
0b10000001, \
0b10000001, \
0b10000001, \
0b11111111))

#define PATTERN_SMALL_DIGIT_0 (LED_CUSTOM_CHAR( \
0b11100000,\
0b10100000,\
0b10100000,\
0b10100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_1 (LED_CUSTOM_CHAR( \
0b00100000,\
0b01100000,\
0b10100000,\
0b00100000,\
0b00100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_2 (LED_CUSTOM_CHAR( \
0b11100000,\
0b00100000,\
0b01000000,\
0b10000000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_3 (LED_CUSTOM_CHAR( \
0b11100000,\
0b00100000,\
0b11100000,\
0b00100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_4 (LED_CUSTOM_CHAR( \
0b10000000,\
0b11000000,\
0b11100000,\
0b01000000,\
0b01000000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_5 (LED_CUSTOM_CHAR( \
0b11100000,\
0b10000000,\
0b11100000,\
0b00100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_6 (LED_CUSTOM_CHAR( \
0b11100000,\
0b10000000,\
0b11100000,\
0b10100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_7 (LED_CUSTOM_CHAR( \
0b11100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_8 (LED_CUSTOM_CHAR( \
0b11100000,\
0b10100000,\
0b11100000,\
0b10100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))

#define PATTERN_SMALL_DIGIT_9 (LED_CUSTOM_CHAR( \
0b11100000,\
0b10100000,\
0b11100000,\
0b00100000,\
0b11100000,\
0b00000000,\
0b00000000,\
0b00000000))


#define PATTERN_LARGE_DIGIT_0 (LED_CUSTOM_CHAR( \
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01110000))

#define PATTERN_LARGE_DIGIT_1 (LED_CUSTOM_CHAR( \
0b00111000,\
0b01001000,\
0b10001000,\
0b00001000,\
0b00001000,\
0b00001000,\
0b00001000,\
0b00001000))

#define PATTERN_LARGE_DIGIT_2 (LED_CUSTOM_CHAR( \
0b01110000,\
0b10001000,\
0b00001000,\
0b00010000,\
0b00100000,\
0b01000000,\
0b10000000,\
0b11111000))

#define PATTERN_LARGE_DIGIT_3 (LED_CUSTOM_CHAR( \
0b01110000,\
0b10001000,\
0b00001000,\
0b00001000,\
0b00110000,\
0b00001000,\
0b10001000,\
0b01110000))

#define PATTERN_LARGE_DIGIT_4 (LED_CUSTOM_CHAR( \
0b10000000,\
0b10000000,\
0b10100000,\
0b10100000,\
0b11111000,\
0b00100000,\
0b00100000,\
0b00100000))

#define PATTERN_LARGE_DIGIT_5 (LED_CUSTOM_CHAR( \
0b11111000,\
0b10000000,\
0b10000000,\
0b11110000,\
0b00001000,\
0b00001000,\
0b10001000,\
0b01110000))

#define PATTERN_LARGE_DIGIT_6 (LED_CUSTOM_CHAR( \
0b00111000,\
0b01000000,\
0b10000000,\
0b10000000,\
0b11110000,\
0b10001000,\
0b10001000,\
0b01110000))

#define PATTERN_LARGE_DIGIT_7 (LED_CUSTOM_CHAR( \
0b11111000,\
0b10001000,\
0b10001000,\
0b00001000,\
0b00010000,\
0b00100000,\
0b01000000,\
0b10000000))

#define PATTERN_LARGE_DIGIT_8 (LED_CUSTOM_CHAR( \
0b01110000,\
0b10001000,\
0b10001000,\
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01110000))

#define PATTERN_LARGE_DIGIT_9 (LED_CUSTOM_CHAR( \
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01111000,\
0b00001000,\
0b10001000,\
0b01110000))

#define PATTERN_HOURGLASS (LED_CUSTOM_CHAR( \
0b01111110,\
0b01000010,\
0b00100100,\
0b00011000,\
0b00011000,\
0b00100100,\
0b01000010,\
0b01111110))

#define PATTERN_TICKS_PER_SECOND (LED_CUSTOM_CHAR( \
0b11111001,\
0b00100010,\
0b00100100,\
0b00101011,\
0b00010100,\
0b00100010,\
0b01000001,\
0b10000110))

// Letters
#define PATTERN_CHAR_A (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b11111000,\
0b10001000,\
0b10001000))

#define PATTERN_CHAR_B (LED_CUSTOM_CHAR( \
0b00000000,\
0b11110000,\
0b10001000,\
0b10001000,\
0b11110000,\
0b10001000,\
0b10001000,\
0b11110000))
  
#define PATTERN_CHAR_C (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b10001000,\
0b10000000,\
0b10000000,\
0b10000000,\
0b10001000,\
0b01110000))

#define PATTERN_CHAR_D (LED_CUSTOM_CHAR( \
0b00000000,\
0b11100000,\
0b10010000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10010000,\
0b11100000))

#define PATTERN_CHAR_E (LED_CUSTOM_CHAR( \
0b00000000,\
0b11111000,\
0b10000000,\
0b10000000,\
0b11110000,\
0b10000000,\
0b10000000,\
0b11111000))

#define PATTERN_CHAR_F (LED_CUSTOM_CHAR( \
0b00000000,\
0b11111000,\
0b10000000,\
0b10000000,\
0b11110000,\
0b10000000,\
0b10000000,\
0b10000000))

#define PATTERN_CHAR_G (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b10001000,\
0b10000000,\
0b10111000,\
0b10001000,\
0b10001000,\
0b01111000))

#define PATTERN_CHAR_H (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b11111000,\
0b10001000,\
0b10001000,\
0b10001000))

#define PATTERN_CHAR_I (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b01110000))

#define PATTERN_CHAR_J (LED_CUSTOM_CHAR( \
0b00000000,\
0b00111000,\
0b00010000,\
0b00010000,\
0b00010000,\
0b00010000,\
0b10010000,\
0b01100000))

#define PATTERN_CHAR_K (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10010000,\
0b10100000,\
0b11000000,\
0b10100000,\
0b10010000,\
0b10001000))

#define PATTERN_CHAR_L (LED_CUSTOM_CHAR( \
0b00000000,\
0b10000000,\
0b10000000,\
0b10000000,\
0b10000000,\
0b10000000,\
0b10000000,\
0b11111000))

#define PATTERN_CHAR_M (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b11011000,\
0b10101000,\
0b10101000,\
0b10001000,\
0b10001000,\
0b10001000))

#define PATTERN_CHAR_N (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b11001000,\
0b10101000,\
0b10011000,\
0b10001000,\
0b10001000))

#define PATTERN_CHAR_O (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01110000))

#define PATTERN_CHAR_P (LED_CUSTOM_CHAR( \
0b00000000,\
0b11110000,\
0b10001000,\
0b10001000,\
0b11110000,\
0b10000000,\
0b10000000,\
0b10000000))

#define PATTERN_CHAR_Q (LED_CUSTOM_CHAR( \
0b00000000,\
0b01110000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10101000,\
0b10010000,\
0b01101000))

#define PATTERN_CHAR_R (LED_CUSTOM_CHAR( \
0b00000000,\
0b11110000,\
0b10001000,\
0b10001000,\
0b11110000,\
0b10100000,\
0b10010000,\
0b10001000))

#define PATTERN_CHAR_S (LED_CUSTOM_CHAR( \
0b00000000,\
0b01111000,\
0b10000000,\
0b10000000,\
0b01110000,\
0b00001000,\
0b00001000,\
0b11110000))

#define PATTERN_CHAR_T (LED_CUSTOM_CHAR( \
0b00000000,\
0b11111000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000,\
0b00100000))

#define PATTERN_CHAR_U (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01110000))

#define PATTERN_CHAR_V (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01010000,\
0b00100000))

#define PATTERN_CHAR_W (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b10101000,\
0b10101000,\
0b01010000))

#define PATTERN_CHAR_X (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b01010000,\
0b00100000,\
0b01010000,\
0b10001000,\
0b10001000))

#define PATTERN_CHAR_Y (LED_CUSTOM_CHAR( \
0b00000000,\
0b10001000,\
0b10001000,\
0b10001000,\
0b01010000,\
0b00100000,\
0b00100000,\
0b00100000))

#define PATTERN_CHAR_Z (LED_CUSTOM_CHAR( \
0b00000000,\
0b11111000,\
0b00001000,\
0b00010000,\
0b00100000,\
0b01000000,\
0b10000000,\
0b11111000))

// - - -

#define PATTERN_ARROW_LEFT (LED_CUSTOM_CHAR( \
0b00010000,\
0b00110000,\
0b01110000,\
0b11111111,\
0b11111111,\
0b01110000,\
0b00110000,\
0b00010000))

#define PATTERN_ARROW_UP (LED_CUSTOM_CHAR( \
0b00011000,\
0b00111100,\
0b01111110,\
0b11111111,\
0b00011000,\
0b00011000,\
0b00011000,\
0b00011000))

#define PATTERN_ARROW_RIGHT (LED_CUSTOM_CHAR( \
0b00001000,\
0b00001100,\
0b00001110,\
0b11111111,\
0b11111111,\
0b00001110,\
0b00001100,\
0b00001000))

#define PATTERN_ARROW_DOWN (LED_CUSTOM_CHAR( \
0b00011000,\
0b00011000,\
0b00011000,\
0b00011000,\
0b11111111,\
0b01111110,\
0b00111100,\
0b00011000))

