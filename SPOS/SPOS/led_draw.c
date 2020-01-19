#include "led_draw.h"
#include "led_patterns.h"
#include "led_paneldriver.h"

// Convention: the coordinates of pixels will be counted from 0, to match its array index in framebuffer

/* Debriefing: In the parameters for draw methods, x should be interpreted as "row" 
 * and y as "column" as per the test tasks. This might be unintuitive, because x would 
 * correspond to the vertical coordinate and vice versa y to the horizontal. 
 * See the picture below:
 * 
 *             -----------> (y, column)
	     |   _______________
		 |  |               |
		 |  |               |
		 |  |   LED panel   |
		 |  |               |
		 |  |_______________|
		 V
	  (x, row)
 */

//! \brief Distributes bits of given color's channels r,g and b unto layers of framebuffer
void draw_setPixel(uint8_t x, uint8_t y, Color color)
{
	/* frame buffer byte pattern: - | - | B2 | G2 | R2 | B1 | G1 | R1 */
	
	uint8_t tmpcolor = 0; // temporary variable to save color for all levels
	
    if(x < 16){ // The row above
		
		for(uint8_t i = 0; i < 3; i++){
			/* Save the (i+1). bit of each r g b (index count from left) */
			tmpcolor  = (color.b >> (5 - i) ) & 0b00000100;
			tmpcolor |= (color.g >> (6 - i) ) & 0b00000010;
			tmpcolor |= (color.r >> (7 - i) ) & 0b00000001;
			
			/* Feed the color to level i */
			framebuffer[i][x][y] &= 0b11111000;
			framebuffer[i][x][y] |= tmpcolor;
		}
		
	} else { // The row below
		
		for(uint8_t i = 0; i < 3; i++){
			/* Save the (i+1). bit of each r g b (index count from left) */
			tmpcolor  = (color.b >> (2 - i) ) & 0b00100000;
			tmpcolor |= (color.g >> (3 - i) ) & 0b00010000;
			tmpcolor |= (color.r >> (4 - i) ) & 0b00001000;
			
			/* Feed the color to level i */
			framebuffer[i][x - 16][y] &= 0b11000111;
			framebuffer[i][x - 16][y] |= tmpcolor;
		}
		
	}
}

//! \brief Reconstructs RGB-Color from layers of frame buffer
Color draw_getPixel(uint8_t x, uint8_t y)
{
	uint8_t tmp_r, tmp_g, tmp_b;
	
	/* frame buffer byte pattern: - | - | B2 | G2 | R2 | B1 | G1 | R1 */
	
    if(x < 16){ // The row above
		
		for(uint8_t i = 0; i < 3; i++){
			/* Take the (i+1). bit of each color based on the byte pattern */
			tmp_r = (framebuffer[i][x][y] & 0b00000001) << (7 - i);
			tmp_g = (framebuffer[i][x][y] & 0b00000010) << (6 - i);
			tmp_b = (framebuffer[i][x][y] & 0b00000100) << (5 - i);
		}
		
	} else { // The row below
		
		for(uint8_t i = 0; i < 3; i++){
			/* Take the (i+1). bit of each color based on the byte pattern */
			tmp_r = (framebuffer[i][x - 16][y] & 0b00001000) << (4 - i);
			tmp_g = (framebuffer[i][x - 16][y] & 0b00010000) << (3 - i);
			tmp_b = (framebuffer[i][x - 16][y] & 0b00100000) << (2 - i);
		}
		
	}
	
	Color color = {.r = tmp_r, .g = tmp_g, .b = tmp_b};
	return color;
}

//! \brief Fills whole panel with given color
void draw_fillPanel(Color color)
{
    for(uint8_t x = 0; x < 32; x++){
		for(uint8_t y = 0; y < 32; y++){
			draw_setPixel(x, y, color);
		}
	}
}

//! \brief Sets every pixel's color to black
void draw_clearDisplay()
{
	Color black = {.r = 0, .g = 0, .b = 0};
    draw_fillPanel(black);
}

//! \brief Draws Rectangle 
void draw_filledRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color)
{
    for(uint8_t row = x1; row <= x2; row++){
		for(uint8_t column = y1; column <= y2; column++){
			draw_setPixel(row, column, color);
		}
	}
}


/*! \brief Draws pattern
 * \param x			Row of left upper corner
 * \param y			Column of left upper corner
 * \param height	Height to be used for the pattern
 * \param width		Width to be used for the pattern
 * \param pattern	The given pattern. Has a maximum span of 8x8 pixel
 * \param color		colorcode used to draw the pattern
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
*/
void draw_pattern(uint8_t x, uint8_t y, uint8_t height, uint8_t width, uint64_t pattern, Color color, bool overwrite){
    uint64_t temprow = 0;
    for(uint8_t i =0; i< height; i++) {
        // Select row
        temprow= pattern>> i*8;
        
        for(uint8_t j =0; j< width; j++) {
            if((x+i<32) && (y+j<32) ) {
                if (temprow&(1<<(7-j))){
                    draw_setPixel(x+i,y+j,color);
                }else {
                    if (overwrite) {
                        draw_setPixel(x+i, y+j, (Color){ .r = 0, .g = 0, .b = 0 });
                    }
                }
            }
        }
    }
}

/*! \brief Draws capital letter on panel
 * \param letter    Letter (A-Z, a-z). Lowercase letters will be converted to uppercase.
 * \param x         Row of left upper corner
 * \param y         Column of left upper corner
 * \param color     Color to draw number with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
*/
void draw_letter(char letter, uint8_t x, uint8_t y, Color color, bool overwrite)
{
    uint64_t pattern = 0;
    
    // To upper
    if (letter >= 'a' && letter <= 'z') letter -= 'a' - 'A';
    
    switch (letter)
    {
        case 'A':
            pattern = PATTERN_CHAR_A;
            break;
        case 'B':
            pattern = PATTERN_CHAR_B;
            break;
        case 'C':
            pattern = PATTERN_CHAR_C;
            break;
        case 'D':
            pattern = PATTERN_CHAR_D;
            break;
        case 'E':
            pattern = PATTERN_CHAR_E;
            break;
        case 'F':
            pattern = PATTERN_CHAR_F;
            break;
        case 'G':
            pattern = PATTERN_CHAR_G;
            break;
        case 'H':
            pattern = PATTERN_CHAR_H;
            break;
        case 'I':
            pattern = PATTERN_CHAR_I;
            break;
        case 'J':
            pattern = PATTERN_CHAR_J;
            break;
        case 'K':
            pattern = PATTERN_CHAR_K;
            break;
        case 'L':
            pattern = PATTERN_CHAR_L;
            break;
        case 'M':
            pattern = PATTERN_CHAR_M;
            break;
        case 'N':
            pattern = PATTERN_CHAR_N;
            break;
        case 'O':
            pattern = PATTERN_CHAR_O;
            break;
        case 'P':
            pattern = PATTERN_CHAR_P;
            break;
        case 'Q':
            pattern = PATTERN_CHAR_Q;
            break;
        case 'R':
            pattern = PATTERN_CHAR_R;
            break;
        case 'S':
            pattern = PATTERN_CHAR_S;
            break;
        case 'T':
            pattern = PATTERN_CHAR_T;
            break;
        case 'U':
            pattern = PATTERN_CHAR_U;
            break;
        case 'V':
            pattern = PATTERN_CHAR_V;
            break;
        case 'W':
            pattern = PATTERN_CHAR_W;
            break;
        case 'X':
            pattern = PATTERN_CHAR_X;
            break;
        case 'Y':
            pattern = PATTERN_CHAR_Y;
            break;
        case 'Z':
            pattern = PATTERN_CHAR_Z;
            break;
    }

    draw_pattern(x, y, LED_CHAR_HEIGHT_LARGE, LED_CHAR_WIDTH_LARGE,  pattern, color, overwrite);
}

/*! \brief Draws Decimal (0..9) on panel
 * \param dec       Decimal number (from 0 to 9)
 * \param x         Row of left upper corner
 * \param y         Column of left upper corner
 * \param color     Color to draw number with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 * \param large     Draws large numbers when set to true, otherwise small (small: 5x3 px, large: 8x5 px)
*/
void draw_decimal(uint8_t dec, uint8_t x, uint8_t y, Color color, bool overwrite, bool large)
{
    uint64_t pattern;

    if (large)
    {
        switch(dec)
        {
            case 1:
                pattern = PATTERN_LARGE_DIGIT_1;
                break;
            case 2:
                pattern = PATTERN_LARGE_DIGIT_2;
                break;
            case 3:
                pattern = PATTERN_LARGE_DIGIT_3;
                break;
            case 4:
                pattern = PATTERN_LARGE_DIGIT_4;
                break;
            case 5:
                pattern = PATTERN_LARGE_DIGIT_5;
                break;
            case 6:
                pattern = PATTERN_LARGE_DIGIT_6;
                break;
            case 7:
                pattern = PATTERN_LARGE_DIGIT_7;
                break;
            case 8:
                pattern = PATTERN_LARGE_DIGIT_8;
                break;
            case 9:
                pattern = PATTERN_LARGE_DIGIT_9;
                break;
            case 0:
                pattern = PATTERN_LARGE_DIGIT_0;
                break;
            default:
                pattern = 0;
                break;
        }
    }
    else
    {
        switch(dec)
        {
            case 1:
                pattern = PATTERN_SMALL_DIGIT_1;
                break;
            case 2:
                pattern = PATTERN_SMALL_DIGIT_2;
                break;
            case 3:
                pattern = PATTERN_SMALL_DIGIT_3;
                break;
            case 4:
                pattern = PATTERN_SMALL_DIGIT_4;
                break;
            case 5:
                pattern = PATTERN_SMALL_DIGIT_5;
                break;
            case 6:
                pattern = PATTERN_SMALL_DIGIT_6;
                break;
            case 7:
                pattern = PATTERN_SMALL_DIGIT_7;
                break;
            case 8:
                pattern = PATTERN_SMALL_DIGIT_8;
                break;
            case 9:
                pattern = PATTERN_SMALL_DIGIT_9;
                break;
            case 0:
                pattern = PATTERN_SMALL_DIGIT_0;
                break;
            default:
                pattern = 0;
                break;
        }
    }

    draw_pattern(x, y, LED_CHAR_HEIGHT_LARGE, LED_CHAR_WIDTH_LARGE, pattern, color, overwrite);
}
