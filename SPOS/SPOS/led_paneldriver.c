#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "util.h"
#include "led_paneldriver.h"
#include "defines.h"

// Global variables
uint8_t rowselect = 0;
uint8_t level = 0;
uint8_t counter = 1;

//! \brief Enable compare match interrupts for Timer 1
void panel_startTimer()
{
	sbi(TIMSK1,OCIE1A);
}

//! \brief Disable compare match interrupts for Timer 1
void panel_stopTimer()
{
	cbi(TIMSK1,OCIE1A);
}

//! \brief Initialization function of Timer 1
void panel_initTimer()
{
	// Configuration TCCR1B register
	sbi(TCCR1B,WGM12);  // Clear on timer compare match
	sbi(TCCR1B,CS12);   // Prescaler 256  1
	cbi(TCCR1B,CS11);   // Prescaler 256  0
	cbi(TCCR1B,CS10);   // Prescaler 256  0

	// Output Compare register 256*7 = 1792 tics => interrupt interval approx 0.0896 ms
	OCR1A = 0x0007;
}

//! \brief Initializes used ports of panel
void panel_init()
{
	//#warning IMPLEMENT STH. HERE
	DDRA |= 0b00001111;//fuer Auswahl der Doppelzeilen
	DDRC |= 0b01000011;//CLK, LE, OE konfigurieren
	DDRD |= 0b00111111;//fuer die Auswahl der Farben

	PORTA &= 0b11110000;//Rowselect mit 0 initialisieren
	PORTC &= 0b11111100;//CLK und LE mit 0 initialisieren
	PORTC |= 0b01000000;//OE mit 0 initialisieren (OE invertiert)
	PORTD &= 0b11000000;//Farben mit 0 initialisieren

	panel_initTimer();//Timer fuer ISR initialisieren
}

void panel_latchEnable(void) {
	PORTC |= 0b00000010;//LE = 1
}

void panel_latchDisable(void) {
	PORTC &= 0b11111101;//LE = 0
}

void panel_outputEnable(void) {
	PORTC &= 0b10111111;//OE = 0 (OE invertiert)
}

void panel_outputDisable(void) {
	PORTC |= 0b01000000;//OE = 1 (OE invertiert)
}

void panel_setAddress(uint8_t rowselect) {
	uint8_t row = (rowselect & 0b00001111);
	//PORTA &= 0b11110000;
	PORTA = 0b11110000 | row;//waehlt die Doppelzeile aus
}

void panel_setColor(uint8_t ebene, uint8_t zeile, uint8_t spalte) {//im Text heisst es setOutput aber egal...
	//PORTD &= 0b11000000;//reset
	PORTD = framebuffer[ebene][zeile][spalte];//gespeichertes Farbbyte zum Einlesen uebergeben
}

void panel_CLK(void) {
	PORTC |= 0b00000001;//Clock kurz auf 1...
	PORTC &= 0b11111110;//...und gleich wieder 0 setzen (keine Wartezeit noetig)
}

void setLevel(void) {//fuer Flackerreduktion
	counter += 1;
	if(counter == 8) {
		counter = 1;//reset
	}
	switch (counter) {
		case 1: level = 0; break;
		case 2: level = 0; break;
		case 3: level = 1; break;
		case 4: level = 1; break;
		case 5: level = 0; break;
		case 6: level = 0; break;
		case 7: level = 2; break;
	}
}

//! \brief ISR to refresh LED panel, trigger 1 compare match interrupts
ISR(TIMER1_COMPA_vect) {
		panel_latchDisable();
		panel_outputDisable();
		//1. Doppelzeile auswaehlen
		panel_setAddress(rowselect);
		for(uint8_t i=0; i<32; i++) {//fuer 32 Spalten
			//2. Farbenbyte uebergeben
			panel_setColor(level, rowselect, i); // Note: Fill the last column first, because we're working with shift register
			//3. Bits mit Clock einlesen lassen
			panel_CLK();
		}
		//4. LE und OE aktivieren
		panel_latchEnable();
		panel_outputEnable();
		//5. LE und OE deaktivieren
		
		//6. Vorbereitung fuer den naechsten ISR Aufruf
		rowselect += 1;
		if(rowselect == 16) {
			rowselect = 0;//reset
			setLevel();//wahlt die naechste Ebene aus
		}
}
