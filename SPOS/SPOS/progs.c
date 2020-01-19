//-------------------------------------------------
//          TestSuite: Farbdarstellung
//-------------------------------------------------

#include "os_core.h"
#include "defines.h"
#include "util.h"
#include "lcd.h"
#include "os_process.h"
#include "os_scheduler.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "led_paneldriver.h"
#include "led_draw.h"
#include "led_patterns.h"

#define NUM_COLORS 11

Color colors[NUM_COLORS] = {
	(Color){ .r = 0xFF, .g = 0xFF, .b = 0xFF }, // White
	(Color){ .r = 0xFF, .g = 0x00, .b = 0x00 }, // Red
	(Color){ .r = 0xA0, .g = 0x00, .b = 0x00 }, // Dark  Red
	(Color){ .r = 0x00, .g = 0xFF, .b = 0x00 }, // Green
	(Color){ .r = 0x00, .g = 0xA0, .b = 0x00 }, // Dark  Green
	(Color){ .r = 0x00, .g = 0x00, .b = 0xFF }, // Blue
	(Color){ .r = 0x00, .g = 0x00, .b = 0xA0 }, // Dark  Blue
	(Color){ .r = 0xFF, .g = 0x00, .b = 0xFF }, // Pink
	(Color){ .r = 0xFF, .g = 0xFF, .b = 0x00 }, // Yellow
	(Color){ .r = 0x00, .g = 0xFF, .b = 0xFF }, // Turquoise
	(Color){ .r = 0x00, .g = 0x00, .b = 0x00 }  // Black
};

PROGRAM(1,AUTOSTART){

	os_setSchedulingStrategy(OS_SS_RUN_TO_COMPLETION);
	panel_init();
	panel_initTimer();
	panel_startTimer();
	lcd_clear();
	lcd_line1();
	lcd_writeString("Farbdarstellung");
	lcd_line2();
	lcd_writeString("Phase 1");

	// Phase 1: set all pixels (white)
	for (uint8_t i = 0; i < 32; i++)
	{
		for (uint8_t j = 0; j < 32; j++)
		{
			draw_setPixel(i, j, colors[0]);
		}
	}
	
	for (uint16_t i = 0; i < 500; i++)
	{
		delayMs(10);
	}

	lcd_clear();
	lcd_line1();
	lcd_writeString("Farbdarstellung");
	lcd_line2();
	lcd_writeString("Phase 2");

	// Phase 2: Display available colors
	uint8_t row = 0;
	for (uint8_t i = 0; i < NUM_COLORS; i++)
	{
		for (uint8_t j = 0; j < 32; j++)
		{
			draw_setPixel(row, j, colors[i]);
		}
		row += 1;
	}
	
	for (uint16_t i = 0; i < 500; i++)
	{
		delayMs(10);
	}

	lcd_clear();
	lcd_line1();
	lcd_writeString("Farbdarstellung");
	lcd_line2();
	lcd_writeString("Phase 3");

	// Phase 3: different brightness levels
	for (uint8_t r = 0; r < 15; r++)
	{
		for (uint8_t c = 0; c < 32; c++)
		{
			draw_setPixel(r, c, colors[1]);
		}
	}
	
	for (uint8_t r = 15; r < 32; r++)
	{
		for (uint8_t c = 0; c < 32; c++)
		{
			draw_setPixel(r, c, colors[2]);
		}
	}
	
	for (uint16_t i = 0; i < 500; i++){
		delayMs(10);
	}
	
	// All off
	for (uint8_t r = 0; r < 32; r++)
	{
		for (uint8_t c = 0; c < 32; c++)
		{
			draw_setPixel(r, c, colors[8]);
		}
	}
	panel_stopTimer();
	while(1);
}
