#ifndef _OS_MEMHEAP_DRIVERS_H//vermeidet Mehrfachinkludierung
#define _OS_MEMHEAP_DRIVERS_H

//Includes----------------------------------------------------------------------

#include "os_mem_drivers.h"
#include <stddef.h>//doxygen
#include "defines.h"//doxygen

//Defines-----------------------------------------------------------------------

#define intHeap (&intHeap__)
#define extHeap (&extHeap__)
#define SAVE_DISTANCE 402

//Global Variables--------------------------------------------------------------

//Datatypes---------------------------------------------------------------------

typedef enum AllocStrategy {//Verwendete Allokationsstrategien fuer den Heap
  OS_MEM_FIRST,
  OS_MEM_NEXT,
  OS_MEM_BEST,
  OS_MEM_WORST
} AllocStrategy;

typedef struct {
  MemDriver* driver;//Zeiger auf Speichertreiber
  MemAddr mapstart;//Startadresse des Map-Bereichs
  uint16_t mapsize;//Groesse des Map-Bereichs
  MemAddr usestart;//Startadresse des Use-Bereichs
  uint16_t usesize;//Groesse des Use-Bereichs
  AllocStrategy currentalloc;//Aktuell verwendete Allokationsstrategie
  const char* name;//richtig so? //Name des Heaps (im Flash-Speicher ablegen)
} Heap;

extern Heap intHeap__;//Initialisierung in os_memheap_drivers.c (deswegen extern), muss hier stehen da MemDriver davor definiert werden muss
extern Heap extHeap__;//analog

//Functions---------------------------------------------------------------------

void os_initHeaps(void);//Initialisierung des Heaps

uint8_t os_getHeapListLength(void);//Anzahl existierender Heaps

Heap* os_lookupHeap(uint8_t index);//Zeiger auf Heap zum Index

#endif
