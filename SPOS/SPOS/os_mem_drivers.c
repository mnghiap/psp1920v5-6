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

