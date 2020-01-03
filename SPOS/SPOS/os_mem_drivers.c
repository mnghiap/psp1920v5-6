//Includes----------------------------------------------------------------------

#include "os_mem_drivers.h"
#include "defines.h"//doxygen
#include "os_spi.h"
#include "os_scheduler.h"
#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//Global Variables--------------------------------------------------------------

//Functions---------------------------------------------------------------------

/*
*Initialisierung des internen Speichermediums
*/
void init(void) {
  //nichts tun, MemDriver erwartet diese Funktion (doxygen)
}

/*
*Liest Wert an Adresse addr und gibt diesen zurueck
*/
MemValue read(MemAddr addr) {
  return *((MemValue*)addr);//MemValue ist nur uint16_t, muss explizit zum Pointer gecastet werden
}

/*
*Schreibt Wert value and Adresse addr des Speichermediums
*/
void write(MemAddr addr, MemValue value) {
  *((MemValue*)addr) = value;//MemValue ist nur uint16_t, muss explizit zum Pointer gecastet werden
}

/*
*Initialisierung der Attribute fuer direkten Zugriff auf Speichermedium
*/
MemDriver intSRAM__ = {//doxygen
  .start = 0x0100,//wegen Register (Dokument)
  .size = 0x1000,//4096Byte (Dokument)
  .init = &init,//Adresse der jeweiligen Funktion (ist ja Pointer)
  .read = &read,//Adresse der jeweiligen Funktion (ist ja Pointer)
  .write = &write//Adresse der jeweiligen Funktion (ist ja Pointer)
};

/*
*Initialisierung des externen Speichermediums
*/
void initExt(void) {
  //DDRB = 0b10110000;//CK, MOSI, CS Ausgang, MISO Eingang (Ports steuert SPI selber)
  DDRB |= 0b10000000;//CK Ausgang
  DDRB &= 0b10111111;//SO Eingang
  DDRB |= 0b00100000;//SI Ausgang
  DDRB |= 0b00010000;//CS Ausgang

  PORTB |= 0b00010000;//CS deaktivieren (akcitve-low), manuell
  PORTB |= 0b01000000;//SO Pull-Up-Widerstand aktivieren
  
  os_spi_init();//Treiber fuer SPI-Modul aktivieren
  //byteweise Zugriffsmodus einstellen
  os_spi_send(0b00000001);//Mode Register einstellen
  os_spi_send(0b00000000);//Byte Operation auswaehlen
}

/*
*Liest Wert an Adresse addr und gibt diesen zurueck
*/
MemValue readExt(MemAddr addr) {
  os_enterCriticalSection();//Schutz
  uint8_t first = 0b00000000;//7 Bit dont cares plus 1 nicht verwendetes Bit wegen 16 Bit statt 17 Bit
  uint8_t second = (uint8_t)(addr >> 8);//ersten 8 Bit von addr
  uint8_t third = (uint8_t)addr;//letzten 8 Bit von addr
  PORTB &= 0b11101111;//CS aktivieren (active-low)
  os_spi_send(0b00000011);//read instruction
  os_spi_send(first);//senden der Addresse
  os_spi_send(second);//senden der Addresse
  os_spi_send(third);//senden der Addresse
  MemValue receive = os_spi_send(0b11111111);//Wert an Adresse auslesen mit Dummy
  PORTB |= 0b00010000;//CS deaktivieren (akcitve-low)
  os_leaveCriticalSection();
  return receive;
}

/*
*Initialisierung der Attribute fuer direkten Zugriff auf Speichermedium
*/
void writeExt(MemAddr addr, MemValue value) {
  os_enterCriticalSection();//Schutz
  uint8_t first = 0b00000000;//7 Bit dont cares plus 1 nicht verwendetes Bit wegen 16 Bit statt 17 Bit
  uint8_t second = (uint8_t)(addr >> 8);//ersten 8 Bit von addr
  uint8_t third = (uint8_t)addr;//letzten 8 Bit von addr
  PORTB &= 0b11101111;//CS aktivieren (active-low)
  os_spi_send(0b00000010);//write instruction
  os_spi_send(first);//senden der Addresse
  os_spi_send(second);//senden der Addresse
  os_spi_send(third);//senden der Addresse
  os_spi_send(value);//schreibt Wert an Adresse
  PORTB |= 0b00010000;//CS deaktivieren (akcitve-low)
  os_leaveCriticalSection();
}

MemDriver extSRAM__ = {
  .start = 0x0000,//Start bei 0
  .size = 0xFFFF,//64KiB
  .init = &initExt,
  .read = &readExt,
  .write = &writeExt
};
