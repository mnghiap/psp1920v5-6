#ifndef _OS_MEM_DRIVERS_H//vermeidet Mehrfachinkludierung
#define _OS_MEM_DRIVERS_H

//Includes----------------------------------------------------------------------

#include <stdint.h>//doxygen

//Defines-----------------------------------------------------------------------

#define intSRAM (&intSRAM__)

//Global Variables--------------------------------------------------------------

//Datatypes---------------------------------------------------------------------

typedef uint16_t MemAddr;//fuer bessere Lesbarkeit
typedef uint8_t MemValue;//fuer bessere Lesbarkeit

typedef struct {//Informationen ueber Speichermedium, direkter Zugriff auf Speichermedium (Untere Schicht)
  MemAddr start;//Startadresse des Speichermediums
  uint16_t size;//Groesse des Speichermediums
  void (*init)(void);//Initialisierung des Speichermediums (Pointer auf Funktion)
  MemValue (*read)(MemAddr addr);//Liest Wert an Adresse (Pointer auf Funktion)
  void (*write)(MemAddr addr, MemValue value);//Schreibt Wert an Adresse (Pointer auf Funktion)
} MemDriver;

extern MemDriver intSRAM__;//Initialisierung in os_mem_drivers.c (deswegen extern), muss hier stehen da MemDriver davor definiert werden muss

//Functions---------------------------------------------------------------------

//Funktionen sind private :P

#endif
