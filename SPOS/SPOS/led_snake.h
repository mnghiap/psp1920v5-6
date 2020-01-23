/*
 * led_snake.h
 *
 * Created: 23.01.2020 13:32:51
 *  Author: Minh Nghia Phan
 */ 

#ifndef LED_SNAKE_H_
#define LED_SNAKE_H_

#include <stdint.h>

typedef struct Position{
	uint8_t x;
	uint8_t y;
} Position;

void ledSnake_startNewGame();

#endif /* LED_SNAKE_H_ */