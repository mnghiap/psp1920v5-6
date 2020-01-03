//Includes----------------------------------------------------------------------

#include "os_memory_strategies.h"
#include "os_memory.h"//doxygen
#include "os_scheduler.h"
#include <stdbool.h>

//Global Variables--------------------------------------------------------------
//uint16_t highorlow;//HighNibble = 1, LowNibble = 0
//Functions---------------------------------------------------------------------

/*MemAddr os_memoryFirstFit(Heap* heap, uint16_t size) {
  highorlow = 1;//beginnt Suche bei HighNibble
  uint16_t sizecounter = size;//zahlt die Anzahl an zusammenhaengenden freien Speicherplaetzen
  MemAddr mapaddr = os_getMapStart(heap);
  while((sizecounter != 0) || mapaddr != os_getUseStart(heap)) {//sizecounter = 0 also gefunden, mapaddr = os_getUseStart(heap) also nicht gefunden
    if(highorlow == 1) {//betracht das HighNibble
      highorlow = 0;//fuer naechstes Nibble
      if(os_getHighNibble(heap, mapaddr) == 0) {//Speicherplatz frei
        sizecounter -= 1;
      }
      else {//Speicherplatz belegt
        sizecounter = size;//gefundene Stelle nicht gross genug
      }
    }
    else {//highorlow == 0
      highorlow = 1;//fuer naechstes Nibble
      if(os_getLowNibble(heap, mapaddr) == 0) {//Speicherplatz frei
        sizecounter -= 1;
      }
      else {//Speicherplatz belegt
        sizecounter = size;//gefundene Stelle nicht gross genug
      }
      mapaddr += 1;//naechstes HighNibble ist in der naechsten Adresse
    }
  }
  if(highorlow == 1) {//Korrektur
    mapaddr -= 1;
    highorlow = 0;
  }
  else {
    highorlow = 1;
  }
  if(sizecounter > 0) {//kein zusammenhaengender freier Speicherplatz gefunden
    return 0;
  }
  else {//Speicherplatz gefunden
    return os_setFirstMallocAddress(heap, mapaddr, size, highorlow);
  }
}
*/

MemAddr os_memoryFirstFit(Heap* heap, uint16_t size, MemAddr beginaddr) {
  //MemAddr useaddr = os_getUseStart(heap);
  MemAddr useaddr = beginaddr;
  uint16_t sizecounter = size;//zahlt zusammenhaengenden freien Speicherplatz
  while((sizecounter != 0) && (useaddr <= (os_getUseStart(heap) + (os_getUseSize(heap)-1)))) {//Abbruch bei gefunden oder nicht gefunden
    if(os_getMapEntry(heap, useaddr) == 0) {//Ein freier Platz wurde gefunden
      sizecounter -= 1;
    }
    else {
      sizecounter = size;//nicht gross genug, von neuem weiter suchen
    }
    useaddr += 1;//betracht naechste Adresse
  }
  useaddr -= 1;//Korrektur, weil Erhoehung for Schleifenbedingung geshieht
  if(sizecounter != 0) {//kein Platzgefunden
    return 0;
  }
  else {//Platz gefunden, useaddr ist letzte gefundene freie Addresse
    return useaddr;
  }
}

MemAddr os_memoryNextFit(Heap* heap, uint16_t size, MemAddr lastaddr) {
  MemAddr addr = os_memoryFirstFit(heap, size, lastaddr);
  if(addr != 0) {
    return addr;
  }
  else {
    addr = os_memoryFirstFit(heap, size, os_getUseStart(heap));
    if(addr != 0) {
      return addr;
    }
    else {
      return 0;
    }
  }
}

/*MemAddr os_memoryBestFit(Heap* heap, uint16_t size) {
  uint16_t sizecounter = 0;
  MemAddr foundaddr = 0;
  MemAddr searchaddr = 0;
  uint16_t difference = os_getUseSize(heap);
  for(uint16_t i=os_getUseStart(heap); i<(os_getUseStart(heap)+os_getUseSize(heap)); i++) {
    if(os_getMapEntry(heap, i) == 0) {//einen freien Platz gefunden
      sizecounter += 1;
      if(sizecounter == size) {
        searchaddr = i;
      }
    }
    else {//Platz ist nicht frei
      if((sizecounter - size) < difference) {
        difference = sizecounter - size;
        foundaddr = searchaddr;
      }
      sizecounter = 0;//reset
    }
  }
  return foundaddr;
}*/

MemAddr os_memoryBestFit(Heap* heap, uint16_t size) {
  uint16_t sizecounter = 0;
  MemAddr foundaddr = 0;
  MemAddr searchaddr = 0;
  uint16_t difference = os_getUseSize(heap);
  for(uint16_t i=os_getUseStart(heap); i<(os_getUseStart(heap)+os_getUseSize(heap)); i++) {
    if(os_getMapEntry(heap, i) == 0) {//einen freien Platz gefunden
      sizecounter += 1;
      if(sizecounter == size) {
        searchaddr = i;
      }
    }
    else {//Platz ist nicht frei
      if(((sizecounter - size) < difference) && (sizecounter != 0)) {
        difference = sizecounter - size;
        foundaddr = searchaddr;
      }
      sizecounter = 0;//reset
    }
  }
  if(((sizecounter - size) < difference) && (sizecounter != 0)) {
    difference = sizecounter - size;
    foundaddr = searchaddr;
  }
  return foundaddr;
}

MemAddr os_memoryWorstFit(Heap* heap, uint16_t size) {
  uint16_t sizecounter = 0;
  MemAddr foundaddr = 0;
  MemAddr searchaddr = 0;
  uint16_t difference = 0;
  for(uint16_t i=os_getUseStart(heap); i<(os_getUseStart(heap)+os_getUseSize(heap)); i++) {
    if(os_getMapEntry(heap, i) == 0) {//einen freien Platz gefunden
      sizecounter += 1;
      if(sizecounter == size) {
        searchaddr = i;
      }
    }
    else {//Platz ist nicht frei
      if((((sizecounter - size)+1) > difference) && (sizecounter != 0)) {
        difference = (sizecounter - size)+1;
        foundaddr = searchaddr;
      }
      sizecounter = 0;//reset
    }
  }
  //wegen Ende des Use-Bereichs
  if((((sizecounter - size)+1) > difference) && (sizecounter != 0)) {
    difference = (sizecounter - size)+1;
    foundaddr = searchaddr;
  }
  return foundaddr;
}

//Hilfsfunktionen
/*MemAddr os_setFirstMallocAddress(Heap const* heap, MemAddr addr, uint16_t size, uint8_t highlow) {//bekommt letzte Speicheradresse
  MemAddr address = os_getMapToUseAdress(heap, addr, highlow);
  for(uint8_t i=size; i>1; i--) {//letztes mit ProzessID beschreiben
    os_setMapEntry(heap, address, 0xF);
    address -= 1;
  }
  os_setMapEntry(heap, address, os_getCurrentProc());//letztes bzw. eigendlich erstes Nibble mit ProzessID beschreiben
  return address;
}

MemAddr os_getMapToUseAdress(Heap const* heap, MemAddr addr, uint8_t highlow) {
  MemAddr mapplace = addr;
  if(highlow == 1) {
    mapplace = ((mapplace - os_getMapStart(heap)) * 2);
    return (os_getUseStart(heap) + mapplace);
  }
  else {//highlow == 0
    mapplace = ((mapplace - os_getMapStart(heap)) * 2) + 1;
    return (os_getUseStart(heap) + mapplace);
  }
}*/
