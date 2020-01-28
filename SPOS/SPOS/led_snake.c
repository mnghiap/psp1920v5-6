/*
 * led_snake.c
 *
 * Created: 23.01.2020 13:32:36
 *  Author: Minh Nghia Phan
 */ 

#include "led_snake.h"
#include "joystick.h"
#include <stdlib.h>
#include "led_draw.h"
#include "util.h"
#include <stdbool.h>

/* Playground size will be 26x32, to leave some place to show scores
 * The wall will surround the playground, leaving an effective space
 * of 24x30 for the snake to move.
 *
 * snakeBody will be the ring buffer. There are 2 variables head
 * and tail to access and write on it. Due to the limited size
 * of the playground, there will be no buffer overflow. Each byte
 * represents effectively 4 elements of the buffer. Buffer index will
 * be counted from 0, e.g. the byte snakeBody[0] represents the 0., 1.,
 * 2., 3. elements of the buffer, and so on. "tail" will chase after
 * "head", which represents the actual state of the snake in game.
 *
 * Encoding of directions: (This is not randomly chosen, look at the 1's)
 * 00: DOWN
 * 11: UP
 * 01: RIGHT
 * 10: LEFT
 *
 * snakeHead represent the absolute coordination of the head
 * The same for food. snakeHead will be used to reconstruct
 * the snake graphically. 
 *
 * currentDirection determines the current direction, in which
 * the snake is moving. This makes updating the game state
 * easier.
 */

Position snakeHead;
Direction currentDirection;
uint16_t highScore = 0;
uint16_t currentScore = 0;
uint8_t  snakeBody[256];
Position food;
uint16_t head = 1;
uint16_t tail = 0;
bool justPaused = false;

Color WHITE1  = { .r = 0xFF, .g = 0xFF, .b = 0xFF }; // White
Color RED1   = { .r = 0xFF, .g = 0x00, .b = 0x00 }; // Red
Color GREEN1  = { .r = 0x00, .g = 0xFF, .b = 0x00 }; // Green
Color BLUE1  = { .r = 0x00, .g = 0x00, .b = 0xFF }; // Blue
Color PINK1   = { .r = 0xFF, .g = 0x00, .b = 0xFF }; // Pink
Color YELLOW1 = { .r = 0xFF, .g = 0xFF, .b = 0x00 }; // Yellow
Color CYAN1   = { .r = 0x00, .g = 0xFF, .b = 0xFF }; // Cyan
Color BLACK1  = { .r = 0x00, .g = 0x00, .b = 0x00 }; // Black
	
// These macros will be used to count up or down "head" and "tail".
#define increment(x) ((x + 1) % 1024) // snakeBody contains 1024 snakebits
#define decrement(x) ((x - 1) % 1024)

// Change colors with these macros. Life is colorful.
#define WALL_COLOR           GREEN1
#define PLAYGROUND_COLOR     BLACK1
#define HIGH_SCORE_COLOR     RED1
#define CURRENT_SCORE_COLOR  BLUE1
#define SNAKE_COLOR          WHITE1
#define FOOD_COLOR           YELLOW1

// Show score (up to 9999) using small patterns with (x,y) as upper left corner
void showScore(uint8_t x, uint8_t y, uint16_t score, Color color){
	uint8_t digit[4];
	score = score % 10000;
	for(uint8_t i = 0; i < 4; i++){
		digit[3-i] = score % 10;
		score /= 10;
	}
	for(uint8_t j = 0; j < 4; j++){
		draw_decimal(digit[j], x, y + j*4, color, true, false);
	}
}

// Show lost screen with bad manners. Start a new game upon pressing the joystick button.
void showLostScreen(){
	draw_clearDisplay();
	uint8_t bm = 0;
	Color color_GG, color_EZ, color_NOOB;
	while(!js_getButton()){
		switch(bm){
			case 1:
				color_GG = RED1;
				color_EZ = CYAN1;
				color_NOOB = YELLOW1;
				break;
				
			case 2:
				color_GG = WHITE1;
				color_EZ = YELLOW1;
				color_NOOB = GREEN1;
				break;
				
			default:
				color_GG = GREEN1;
				color_EZ = PINK1;
				color_NOOB = WHITE1;
				break;
		}
		bm = (bm + 1) % 3;
		draw_letter('G', 6, 2, color_GG, true);
		draw_letter('G', 6, 9, color_GG, true);
		draw_letter('E', 6, 18, color_EZ, true);
		draw_letter('Z', 6, 25, color_EZ, true);
		draw_letter('N', 17, 3, color_NOOB, true);
		draw_letter('O', 17, 11, color_NOOB, true);
		draw_letter('O', 17, 18, color_NOOB, true);
		draw_letter('B', 17, 25, color_NOOB, true);
		delayMs(300);
	}
	delayMs(1000);
	ledSnake_startNewGame();
}

// Show pause screen
void showPauseScreen(){
	draw_clearDisplay();
	draw_letter('P', 13, 2, PINK1, true);
	draw_letter('A', 13, 8, BLUE1, true);
	draw_letter('U', 13, 14, RED1, true);
	draw_letter('S', 13, 20, GREEN1, true);
	draw_letter('E', 13, 26, YELLOW1, true);
	while(!js_getButton()){
		// Wait for button to be pressed
	}
	delayMs(1000);
}

// Init graphic of the game
void initDisplay(){
	draw_clearDisplay();
	
	// Show high score in red
	showScore(0, 0, highScore, HIGH_SCORE_COLOR);
	
	// Show current score in blue
	showScore(0, 17, currentScore, CURRENT_SCORE_COLOR);
	
	// Draw the wall in green
	for(uint8_t x = 6; x < 32; x++){
		draw_setPixel(x, 0, WALL_COLOR);
		draw_setPixel(x, 31, WALL_COLOR);
	}
	
	for(uint8_t y = 0; y < 32; y++){
		draw_setPixel(6, y, WALL_COLOR);
		draw_setPixel(31, y, WALL_COLOR);
	}
	
	/* In general, when drawing food or snake, their x coordinate is shifted by +6
	 * so we can show the score above.
	 */
}
// Forward declarations
Direction snake_get(uint16_t pos);

// State of the game will be reconstructed graphically and output.
// This includes only the snake and the food.
void updateDisplay(){
	// Clear the playground
	draw_filledRectangle(7, 1, 30, 30, PLAYGROUND_COLOR);
	
	// Reconstruct the snake from the position of the head and the ring buffer
	draw_setPixel(snakeHead.x + 6, snakeHead.y, SNAKE_COLOR);
	Position tmp_pos = {.x = snakeHead.x, .y = snakeHead.y};
	for(uint16_t i = decrement(head); i != decrement(tail); i = decrement(i)){
		Direction tmp_dir = snake_get(i);
		switch(tmp_dir){
			case JS_DOWN: tmp_pos.x--; break;
			case JS_UP: tmp_pos.x++; break;
			case JS_LEFT: tmp_pos.y++; break;
			case JS_RIGHT: tmp_pos.y--; break;
			default: break;
		}
		draw_setPixel(tmp_pos.x + 6, tmp_pos.y, SNAKE_COLOR);
	}
	
	// Draw food
	draw_setPixel(food.x + 6, food.y, FOOD_COLOR);
	
}

// Function to encode the direction to 2 bits
uint8_t encodeDirection(Direction dir){
	switch(dir){
		case JS_DOWN: return 0b00; break;
		case JS_UP: return 0b11; break;
		case JS_LEFT: return 0b10; break;
		case JS_RIGHT: return 0b01; break;
		default: return 0; break;
	}
}

// Function to decode 2 bits to the corresponding direction
Direction decodeDirection(uint8_t encoded){
	switch(encoded){
		case 0b00: return JS_DOWN; break;
		case 0b01: return JS_RIGHT; break;
		case 0b10: return JS_LEFT; break;
		case 0b11: return JS_UP; break;
		default: return JS_DOWN; break;
	}
}

// Get the direction of the pos. element of the snakeBody
Direction snake_get(uint16_t pos){
	uint8_t bytePos = pos / 4; // which byte in the array snakeBody
	uint8_t innerPos = pos % 4; // which 2 bits in the byte above
	uint8_t encodedDirection = 0;
	switch(innerPos){
		case 0: encodedDirection = (snakeBody[bytePos] & 0b11000000) >> 6; break;
		case 1: encodedDirection = (snakeBody[bytePos] & 0b00110000) >> 4; break;
		case 2: encodedDirection = (snakeBody[bytePos] & 0b00001100) >> 2; break;
		case 3: encodedDirection = (snakeBody[bytePos] & 0b00000011) >> 0; break;
		default: break;
	}
	return decodeDirection(encodedDirection);
}

// Set the direction of the pos. element to dir
void snake_set(uint16_t pos, Direction dir){
	uint8_t bytePos = pos / 4;
	uint8_t innerPos = pos % 4;
	uint8_t encodedDirection = encodeDirection(dir);
	switch(innerPos){
		case 0: 
			snakeBody[bytePos] &= 0b00111111;
			snakeBody[bytePos] |= (encodedDirection << 6);
			break;
			
		case 1:
			snakeBody[bytePos] &= 0b11001111;
			snakeBody[bytePos] |= (encodedDirection << 4);
			break;
		
		case 2:
			snakeBody[bytePos] &= 0b11110011;
			snakeBody[bytePos] |= (encodedDirection << 2);
			break;
		
		case 3:
			snakeBody[bytePos] &= 0b11111100;
			snakeBody[bytePos] |= (encodedDirection << 0);
			break;
			
		default: break;
	}
}

// Add the direction of the next snakebit to the current position of head
void snake_addHead(Direction dir){
	snake_set(head, dir);
	head = increment(head);
}

// Remove the tail of the snake
void snake_removeTail(){
	tail = increment(tail);
}

// Find new position for food
void resetFoodPosition(){
	food.x = 1 + (rand() % 24); // Make sure the coordinates are between 1 and 24
	food.y = 1 + (rand() % 24);
}

void resetSnakePosition(){
	snakeHead.x = 12;
	snakeHead.y = 15;
	currentDirection = JS_LEFT;
	head = 1;
	tail = 0;
	snake_set(tail, JS_LEFT);
}

// Reset the state of the game. Called whenever a new game is started
void resetGame(){
	// Reset current score
	currentScore = 0;
	
	// Set the position of food
	resetFoodPosition();
	
	// Set the position of head and the tail
	resetSnakePosition();
	
	// Re-initialize game screen
	initDisplay();
}

// This function does the necessaries when a game is lost
void loseGame(){
	if(currentScore > highScore){
		highScore = currentScore; // Save high score
	}
	showLostScreen(); // Show lost screen
}

// Main function to update game state. Display will be updated here directly.
void updateGameState(){
	if(js_getButton()){
		/* Show pause screen if the button is pressed*/
		showPauseScreen();
		justPaused = true;
	} else {
		/* Read the input from the joystick*/
		Direction nextDir = js_getDirection();
		
		/* Direction of the snake won't be change if nextDir is JS_NEUTRAL
		 * or nextDir is the opposite direction of the current one.
		 */
		
		if( nextDir == JS_NEUTRAL ||
		   (nextDir == JS_LEFT && currentDirection == JS_RIGHT) ||
		   (nextDir == JS_RIGHT && currentDirection == JS_LEFT) ||
		   (nextDir == JS_DOWN && currentDirection == JS_UP) ||
		   (nextDir == JS_UP && currentDirection == JS_DOWN) ){
			   nextDir = currentDirection;
		}
		
		/* Update the current direction of the snake */
		currentDirection = nextDir;
		
		/* Update the current position of the head */
		switch(nextDir){
			case JS_LEFT: snakeHead.y--; break;
			case JS_RIGHT: snakeHead.y++; break;
			case JS_UP: snakeHead.x--; break;
			case JS_DOWN: snakeHead.x++; break;
			default: break;
		}
		
		/* Update the ring buffer */
		snake_addHead(nextDir);
		
		/* Check for collision with wall */
		if (snakeHead.x == 0 || snakeHead.x == 25 || snakeHead.y == 0 || snakeHead.y == 31){
			loseGame();
		}
			
		/* Check for self collision
		 * This will be done by calculating the coordinations of all snakebits
		 * and compare with the head. We will make use of the directions saved
		 * in snakeBody.
		 */
		Position tmp_pos = {.x = snakeHead.x, .y = snakeHead.y}; // temporary variable to store temporary coordinate
		for(uint16_t i = decrement(head); i != decrement(tail); i = decrement(i)){ 
			// This loop will calculate a snakebit coordinate based on its successor
			Direction dir = snake_get(i);
			switch(dir){
				case JS_DOWN: tmp_pos.x--; break;
				case JS_UP: tmp_pos.x++; break;
				case JS_LEFT: tmp_pos.y++; break;
				case JS_RIGHT: tmp_pos.y--; break;
				default: break;
			}
			// Compare with the head
			if(tmp_pos.x == snakeHead.x && tmp_pos.y == snakeHead.y){
				loseGame();
			}
		}
		
		/* Check whether food was eaten */
		if(snakeHead.x == food.x && snakeHead.y == food.y){ // If food was eaten...
			resetFoodPosition(); //... then make new food
			currentScore++; // and gain a score	
			showScore(0, 17, currentScore, CURRENT_SCORE_COLOR); // show the new score
			
			/* Apparently, showScore overwrites some parts of the upper wall. 
			 * We have to draw them again.
			 */
			for(uint8_t col = 0; col < 32; col++){
				draw_setPixel(6, col, WALL_COLOR);
			}
			draw_setPixel(7, 31, WALL_COLOR);
			// The tail doesn't change
		} else { // Position of the tail changes
			snake_removeTail(); // Remove the tail from the ring buffer
		}
		
		/* justPaused will determine whether we have to init the screen again */
		if(justPaused == true){
			initDisplay();
		}
		updateDisplay();
		justPaused = false;
		delayMs(400); // Give player reaction time. Can me modified to set difficulty.
	}
		
}

// Function to init the game
void ledSnake_startNewGame(){
	resetGame();
	
	while(1){
		updateGameState();	
	}
}