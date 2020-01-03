#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>

/*! \file

Everything that is necessary to get the input from the Buttons in a clean format.

*/

/*!
 *  A simple "Getter"-Function for the Buttons on the evaluation board.\n
 *
 *  \returns The state of the button(s) in the lower bits of the return value.\n
 *  example: 1 Button:  -pushed:   00000001
 *                      -released: 00000000
 *           4 Buttons: 1,3,4 -pushed: 000001101
 *
 */
uint8_t os_getInput(void) {//1=Enter 2=Down 3=Up 4=ESC
    //#warning IMPLEMENT STH. HERE
    uint8_t buttons = ~PINC;
    buttons &= 0b11000011;
    buttons |= (buttons >> 4);
    return (buttons & 0b00001111);
}

/*!
 *  Initializes DDR and PORT for input
 */
void os_initInput() {
    //#warning IMPLEMENT STH. HERE
    DDRC &= 0b00000000;
    PORTC |= 0b11111111;
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
    //#warning IMPLEMENT STH. HERE
    while(os_getInput() != 0b00000000) {
    }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
    //#warning IMPLEMENT STH. HERE
    while(os_getInput() == 0b00000000) {
    }
}
