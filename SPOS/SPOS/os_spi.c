//Includes----------------------------------------------------------------------

#include "os_spi.h"
#include "os_scheduler.h"

//Global Variables--------------------------------------------------------------

//Functions---------------------------------------------------------------------

void os_spi_init(void) {
  //Konfiguration fuer Bus relevanten I/O-Pins


  //SPI-Modul und Master aktivieren, SPI-Taktfrequenz einstellen
  //SPCR = 0b01010000;
  SPCR |= 0b00010000;//Master aktivieren (MSTR)
  SPCR &= 0b01111111;//ISR ausschalten (SPIE)
  SPCR &= 0b11011111;//MSB first (DORD)
  SPCR |= 0b00001000;//CPOL
  SPCR |= 0b00000100;//CPHA
  SPCR &= 0b11111101;//SPR1
  SPCR &= 0b11111110;//SPR0
  SPCR |= 0b01000000;//SPI-Modul aktivieren (SPE)

  SPSR &= 0b01111111;//SPIF
  SPSR |= 0b00000001;//SPI2X
}

uint8_t os_spi_send(uint8_t data) {
  os_enterCriticalSection();//Uebertragung darf nicht gestoert werden
  SPDR = data;//zu sendende Daten uebertragen
  while((SPSR >> 7) == 0) {//aktiv warten bis SPIF Bit 1 ist fuer Ende der Uebertragung
  }
  os_leaveCriticalSection();
  return SPDR;//gebe empfangene Daten zurueck (fuer receive)
}

uint8_t os_spi_receive() {
  return os_spi_send(0b11111111);//Nutze send aus, uebergebe Dummy-Byte
}
