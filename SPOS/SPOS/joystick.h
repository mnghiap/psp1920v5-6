/*
 * joystick.h
 *
 * Created: 18.01.2020 00:05:34
 *  Author: iw851247
 */ 

#include <stdbool.h>
#include <stdint.h>

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

// Enum to represent direction of the joystick
typedef enum Direction {
	JS_LEFT,
	JS_RIGHT,
	JS_UP,
	JS_DOWN,
	JS_NEUTRAL
} Direction;

void js_init();

uint16_t js_getHorizontal();

uint16_t js_getVertical();

Direction js_getDirection();

bool js_getButton();

#endif /* JOYSTICK_H_ */