#ifndef _OS_MEMORY_STRATEGIES_H
#define _OS_MEMORY_STRATEGIES_H

//Includes----------------------------------------------------------------------

#include "os_memheap_drivers.h"//doxygen

//Defines-----------------------------------------------------------------------

//Global Variables--------------------------------------------------------------

//Datatypes---------------------------------------------------------------------

//Functions---------------------------------------------------------------------

MemAddr os_memoryFirstFit(Heap* heap, uint16_t size,MemAddr beginaddr);

MemAddr os_memoryNextFit(Heap* heap, uint16_t size, MemAddr lastaddr);

MemAddr os_memoryBestFit(Heap* heap, uint16_t size);

MemAddr os_memoryWorstFit(Heap* heap, uint16_t size);

MemAddr os_setFirstMallocAddress(Heap const* heap, MemAddr addr, uint16_t size, uint8_t highlow);

MemAddr os_getMapToUseAdress(Heap const* heap, MemAddr addr, uint8_t highlow);

#endif
