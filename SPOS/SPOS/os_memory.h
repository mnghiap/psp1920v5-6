#ifndef _OS_MEMORY_H
#define _OS_MEMORY_H

//Includes----------------------------------------------------------------------

#include "os_mem_drivers.h"//doxygen
#include "os_memheap_drivers.h"//doxygen
#include "os_scheduler.h"//doxygen

//Defines-----------------------------------------------------------------------

//Global Variables--------------------------------------------------------------

//Datatypes---------------------------------------------------------------------

//Functions---------------------------------------------------------------------

/*
*Soll im heap einen freien zusammenhaengenden Speicherplatz von der Groesse mindestens size allokieren
*gibt 0 zurueck, falls kein genuegend grosser Speicherplatz gefunden wurde
*Rueckgabewertist erstes alloziertes Byte im Use-Bereich
*/
MemAddr os_malloc(Heap* heap, uint16_t size);

/*
*verkleinert bzw. vergroessert Groesse eines Speicherbereichs dynamisch
*/
MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size);

/*
*Bekommt Use-Bereich Adresse
*Soll entsprechende im Map-Bereich markierter allozierter Speicherbereich wieder freigeben
*Speicher darf nur von seinem Besitzer-Prozess freigegeben werden, sonst Fehlermeldung
*/
void os_free(Heap* heap, MemAddr addr);

/*
*gibt alle von pid allozierten Speicherbereiche auf dem heap frei
*/
void os_freeProcessMemory(Heap* heap, ProcessID pid);

size_t os_getMapSize(Heap const* heap);

size_t os_getUseSize(Heap const* heap);

MemAddr os_getMapStart(Heap const* heap);

MemAddr os_getUseStart(Heap const* heap);

/*
*Liefert Groesse in Byte eines belegten Speicherabschnitts (Chunk)
*Falls Speicherabschnitt frei muss 0 zurueckgegeben werden
*/
uint16_t os_getChunkSize(Heap const* heap, MemAddr addr);

AllocStrategy os_getAllocationStrategy(Heap const* heap);

void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat);

/*
*Gibt die erste reservierte Adresse des Chunks wieder (Use-Bereich)
*nur aufrufen wenn bereits geprueft wurde, dass der Chunk nicht frei ist
*/
MemAddr os_firstChunkAddress(Heap const* heap, MemAddr addr);

/*
*bekommt eine Use-Bereich Adresse und gibt das dazugehoerige Verwaltungs-Nibble zurueck
*/
MemValue os_getMapEntry(Heap const* heap, MemAddr addr);

/*
*bekommt eine Use-Bereich Adresse und setzt das dazugehoerige Verwaltungs-Nibble
*/
void os_setMapEntry(Heap const* heap, MemAddr addr, MemValue value);

//Bekommt Adresse im Map-Bereich und gibt/setzt das entsprechende Nibble
MemValue os_getHighNibble(Heap const* heap, MemAddr addr);

MemValue os_getLowNibble(Heap const* heap, MemAddr addr);

void os_setHighNibble(Heap const* heap, MemAddr addr, MemValue value);

void os_setLowNibble(Heap const* heap, MemAddr addr, MemValue value);

/*
*allokiert gemeinsamen Speicherbereich analog zu malloc aber mit neuem Protokoll
*/
MemAddr os_sh_malloc(Heap* heap, uint16_t size);

/*
*gibt gemeinsamen Speicherbereich frei analog zu free
*/
void os_sh_free(Heap* heap, MemAddr* ptr);

/*
*oeffnet Speicherbereich fuer lesenden Zugriff
*/
MemAddr os_sh_readOpen(Heap const* heap, MemAddr const *ptr);

/*
*oeffnet Speicherbereich fuer schreibenden Zugriff
*/
MemAddr os_sh_writeOpen(Heap const* heap, MemAddr const *ptr);

/*
*schliesst Speicherbereich wieder
*/
void os_sh_close(Heap const* heap, MemAddr addr);

/*
*schreibender Zugriff auf Speicherbereich, indem Daten in den gemeinsamen Speicherbereich kopiert werden
*/
void os_sh_write(Heap const* heap, MemAddr const* ptr, uint16_t offset, MemValue const* dataSrc, uint16_t length);

/*
*lesender Zugriff auf Speicherbereich, indem Daten aus dem gemeinsamen Speicherbereich kopiert werden
*/
void os_sh_read(Heap const* heap, MemAddr const* ptr, uint16_t offset, MemValue* dataDest, uint16_t length);

#endif
