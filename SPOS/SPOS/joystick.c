/*
 * joystick.c
 *
 * Created: 18.01.2020 00:05:10
 *  Author: iw851247
 */ 

#include "joystick.h"
#include <avr/io.h>
#include <stdlib.h>

void js_init(){
	/* We need to configure the ADMUX and ADCSRA register */
	
	// ADMUX = REFS1 | REFS0 | ADLAR | MUX4 | MUX3 | MUX2 | MUX1 | MUX0
	// REFS1 = 0, REFS0 = 1 to set Vcc as reference voltage
	// ADLAR = 0 so the result won't be left shifted
	// MUX4 to 0 will be specifically set later based on joystick direction
	
	ADMUX = 0b01000000;
	
	// ADCSRA = ADEN | ADSC | - | - | - | - | - | -
	// ADEN = 1 to enable the internal ADC
	// ADSC = 0 as there's nothing yet to convert
	// The last 3 bits are set to get the right frequency
	// We are not interested in other bits
	
	ADCSRA |= 0b10000111;
	
	DDRA &= 0b00011111; // configure PINA5:6:7 as input
	PORTA |= 0b11100000; // Pull-up
}

uint16_t js_getHorizontal(){
	/* Start an AD conversion from PINA5 */
	
	ADMUX = 0b01000101; // Set MUX4:0 as 00101 to choose PINA5 as input channel
	
	ADCSRA |= (1 << ADSC); // Set ADSC bit to start conversion
	
	while((ADCSRA & (1 << ADSC)) != 0){ 
		// Wait until the ADSC bit becomes 0 
	}
	
	uint16_t result = ((uint16_t)ADCL | (uint16_t)ADCH << 8);
	
	return result;
}

uint16_t js_getVertical(){
	/* Start an AD conversion from PINA6 */
	
	ADMUX = 0b01000110; // Set MUX4:0 as 00110 to choose PINA6 as input channel
	
	ADCSRA |= (1 << ADSC); // Set ADSC bit to start conversion
	
	while((ADCSRA & (1 << ADSC)) != 0){
		// Wait until the ADSC bit becomes 0
	}
	
	uint16_t result = ((uint16_t)ADCL | (uint16_t)ADCH << 8);
	
	return result;
}

// Macros to make life easier. See the comments in js_getDirection()

#define low(x)      (  (0 <= x) && (x <= 409))
#define neutral(x)  ((410 <= x) && (x <= 614))
#define high(x)     ((615 <= x) && (x <= 1023))
#define lowEdge(x)  (x)
#define highEdge(x) (1023 - x)

Direction js_getDirection(){
	/* For the neutral position (2.5V) there should be a tolerance of 1.0V, 
	 * which means for an axis:
	 * 0.0V - 2.0V represents the low (x: left, y: up) position
	 * 2.0V - 3.0V represents the neutral position
	 * 3.0V - 5.0V represents the high (x: right, y: down) position
	 *
	 * From which we have the number intervals that represent
	 * the position of an axis:
	 * [  0,  409]: low
	 * [410,  614]: neutral
	 * [615, 1023]: high
	 * 
	 * So there are 3x3=9 cases
	 */
	
	uint16_t horizontal = js_getHorizontal();
	uint16_t vertical = js_getVertical();
	
	if(neutral(horizontal) && neutral(vertical)){
		return JS_NEUTRAL;
	}
	
	if(neutral(horizontal) && low(vertical)){
		return JS_UP;
	}
	
	if(neutral(horizontal) && high(vertical)){
		return JS_DOWN;
	}
	
	if(low(horizontal) && neutral(vertical)){
		return JS_LEFT;
	}
	
	if(high(horizontal) && neutral(vertical)){
		return JS_RIGHT;
	}
	
	// For the rest cases the direction will be determined based on
	// which one of horizontal or vertical is closer to the "edge" (0 for low, 1023 for high)
	
	if(low(horizontal) && low(vertical)){
		if(lowEdge(horizontal) < lowEdge(vertical)){
			return JS_LEFT;
		} else return JS_UP;
	}
	
	if(low(horizontal) && high(vertical)){
		if(lowEdge(horizontal) < highEdge(vertical)){
			return JS_LEFT;
		} else return JS_DOWN;
	}
	
	if(high(horizontal) && low(vertical)){
		if(highEdge(horizontal) < lowEdge(vertical)){
			return JS_RIGHT;
		} else return JS_UP;
	}
	
	if(high(horizontal) && high(vertical)){
		if(highEdge(horizontal) < highEdge(vertical)){
			return JS_RIGHT;
		} else return JS_DOWN;
	}
	
	return JS_NEUTRAL; 
}

bool js_getButton(){
	/* Button is pressed if PINA7 = 0 */
	uint8_t button = (~PINA >> 7);
	return (button == 1);
}