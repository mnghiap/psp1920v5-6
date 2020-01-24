#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Color;

//! Used by Testtask to draw Arrow Symbol
void draw_pattern(uint8_t x, uint8_t y, uint8_t height, uint8_t width, uint64_t pattern, Color color, bool overwrite);

//! Draws Decimal (0..9) on panel
void draw_decimal(uint8_t dec, uint8_t x, uint8_t y, Color color, bool overwrite, bool large);

//! Draws capital letter on panel
void draw_letter(char letter, uint8_t x, uint8_t y, Color color, bool overwrite);

void draw_setPixel(uint8_t x, uint8_t y, Color color);
Color draw_getPixel(uint8_t x, uint8_t y);
void draw_fillPanel(Color color);
void draw_clearDisplay();
void draw_filledRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color);
