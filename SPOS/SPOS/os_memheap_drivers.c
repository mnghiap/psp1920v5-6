//Includes----------------------------------------------------------------------

#include "os_memheap_drivers.h"
#include "defines.h"//doxygen
#include "os_memory_strategies.h"//doxygen
#include <avr/pgmspace.h>//evtl noetig wegen PROGMEM (doxygen)

//Global Variables--------------------------------------------------------------

static char const PROGMEM intStr[] = "internal";//doxygen
static char const PROGMEM extStr[] = "external";//doxygen

//Functions---------------------------------------------------------------------

/*
*Initialisierung des Heaps
*/
void os_initHeaps(void) {
  for(uint16_t i=(intHeap->mapstart); i<(intHeap->usestart); i++) {
    intHeap->driver->write(i, 0);//Initialisiert alle Werte im Map-Bereich mit 0
  }
  for(uint16_t j=(extHeap->mapstart); j<((extHeap->usestart)+(extHeap->usesize)); j++) {
    extHeap->driver->write(j, 0);//Initialisiert alle Werte im Map-Bereich mit 0
  }
}

/*
*Gibt Anzahl der existierenden Heaps zurueck
*/
uint8_t os_getHeapListLength(void) {
  return 2;//intHeap und extHeap
}

/*
*Gibt Zeiger auf Heap des uebergebenen Index zurueck
*/
Heap* os_lookupHeap(uint8_t index) {
  if(index == 0) {//Interner Heap hat Index 0
    return intHeap;
  }
  else {//Externer heap hat Index 1
    return extHeap;
  }
}

/*
*Initialisierung der Attribute fuer Manipulation des Speichermedium
*/
Heap intHeap__ = {//richtig so?Gehe davon aus der Rest wegen Division ohne Rest wird einfach nicht genutzt
  .driver = intSRAM,
  .mapstart = 0x0100 + SAVE_DISTANCE,
  .mapsize = ((0x1000 / 2)-SAVE_DISTANCE) / 3,//Haelfte fuer Heap, Verhaeltnis 1:2 Map-Bereich zu Use-Bereich
  .usestart = 0x0100 + SAVE_DISTANCE + (((0x1000 / 2)-SAVE_DISTANCE) / 3),
  .usesize = (((0x1000 / 2)-SAVE_DISTANCE) / 3) * 2,//Verhaeltnis 1:2 Map-Bereich zu Use-Bereich
  .currentalloc = OS_MEM_FIRST,//fuer diesen Versuch
  .name = intStr//Zeiger auf Name des Heaps (siehe oben)
};

Heap extHeap__ = {
  .driver = extSRAM,
  .mapstart = 0x0000,
  .mapsize = 0xFFFF / 3,
  .usestart = 0x0000 + (0xFFFF / 3),
  .usesize = (0xFFFF / 3) * 2,
  .currentalloc = OS_MEM_FIRST,
  .name = extStr
};
