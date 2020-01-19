/*! \file 
 *  \brief Functions to draw premade things on the LED Panel 
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2019
 */
#pragma once
#include <avr/io.h>
#include <stdbool.h>

//! Make framebuffer public for draw methods 
uint8_t framebuffer[3][16][32];//Ebene-Doppelzeile-Spalte


//! Initializes registers
void panel_init();

//! Starts interrupts
void panel_startTimer(void);

//! Stops interrupts
void panel_stopTimer(void);

//! Initalizes interrupt timer
void panel_initTimer(void);
