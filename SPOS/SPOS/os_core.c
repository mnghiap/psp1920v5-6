#include "os_core.h"
#include "defines.h"
#include "util.h"
#include "lcd.h"
#include "os_input.h"
//geaendert
#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"

#include <avr/interrupt.h>

void os_initScheduler(void);

/*! \file
 *
 * The main system core with initialization functions and error handling.
 *
 */

/*
 * Certain key-functionalities of SPOS do not work properly if optimization is
 * disabled (O0). It will still compile with O0, but you may encounter
 * run-time errors. This check is supposed to remind you on that.
 */
#ifndef __OPTIMIZE__
    #warning "Compiler optimizations disabled; SPOS and Testtasks may not work properly."
#endif

/*!
 *  Initializes the used timers.
 */
void os_init_timer(void) {
    // Init timer 2 (Scheduler)
    sbi(TCCR2A, WGM21); // Clear on timer compare match

    sbi(TCCR2B, CS22); // Prescaler 1024  1
    sbi(TCCR2B, CS21); // Prescaler 1024  1
    sbi(TCCR2B, CS20); // Prescaler 1024  1
    sbi(TIMSK2, OCIE2A); // Enable interrupt
    OCR2A = 60;

    // Init timer 0 with prescaler 256
    cbi(TCCR0B, CS00);
    cbi(TCCR0B, CS01);
    sbi(TCCR0B, CS02);

    sbi(TIMSK0, TOIE0);
}

/*!
 *  Readies stack, scheduler and heap for first use. Additionally, the LCD is initialized. In order to do those tasks,
 *  it calls the sub function os_initScheduler().
 */
void os_init(void) {
    // Init timer 0 and 2
    os_init_timer();

    // Init buttons
    os_initInput();

    // Init LCD display
    lcd_init();

    lcd_writeProgString(PSTR("Booting SPOS ..."));

    os_initScheduler();

    os_coarseSystemTime = 0;

    //geaendert
    intSRAM->init();
    extSRAM->init();
    os_initHeaps();
}

/*!
 *  Terminates the OS and displays a corresponding error on the LCD.
 *
 *  \param str  The error to be displayed
 */
void os_errorPStr(char const* str) {
    //#warning IMPLEMENT STH. HERE
    uint8_t active = SREG;//merkt sich Zustand vom Interrupt
    SREG &= 0b01111111;//Betriebssystem anhalten (Interrupts global deaktivieren)
    lcd_clear();//Display leeren
    lcd_writeProgString(str);//Fehlermeldung auf LCD ausgeben
    while((os_getInput() & 0b00001001) != 0b00001001) {//solange warten bis "Enter+ESC" gedrueckt wurde
    }
    lcd_clear();//Fehlermeldung beenden
    if((active & 0b10000000) == 0b10000000) {//fragt ob vorher Interrupt aktiviert war
    SREG |= 0b10000000;//Betriebssystem fortfuehren (Interrupts global aktivieren)
    }
}
