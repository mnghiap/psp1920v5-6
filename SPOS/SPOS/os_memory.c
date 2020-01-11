//Includes----------------------------------------------------------------------

#include "os_memory.h"
#include "os_memory_strategies.h"//doxygen
#include "util.h"//doxygen
#include "os_core.h"//doxygen
#include <stdbool.h>
#include "os_memheap_drivers.h"

//Global Variables--------------------------------------------------------------
Heap* currentheap;//merkt sich den gerade verwendeten Heap
MemAddr nextaddrint;//merkt sich die letzte gefundene Adresse im Use-Bereich
MemAddr nextaddrext;
bool initializerint = true;//fuer Initialisierung von nextaddr
bool initializerext = true;
//Functions---------------------------------------------------------------------

MemAddr os_malloc(Heap* heap, uint16_t size) {
  os_enterCriticalSection();//Betreten des kritischen Bereichs
  if(initializerint && (heap == intHeap)) {
    nextaddrint = os_getUseStart(heap)-1;
    initializerint = false;
  }
  if(initializerext && (heap == extHeap)) {
    nextaddrext = os_getUseStart(heap)-1;
    initializerext = false;
  }
  MemAddr useaddr = 0;//irgendwie initialisieren
  switch(heap->currentalloc) {
    case OS_MEM_FIRST: useaddr = os_memoryFirstFit(heap, size, os_getUseStart(heap)); break;
    case OS_MEM_NEXT:
    if(heap == intHeap) {
      useaddr = os_memoryNextFit(heap, size, nextaddrint+1);
    }
    if(heap == extHeap) {
      useaddr = os_memoryNextFit(heap, size, nextaddrext+1);
    }
    break;
    case OS_MEM_BEST: useaddr = os_memoryBestFit(heap, size); break;
    case OS_MEM_WORST: useaddr = os_memoryWorstFit(heap, size); break;
  }
  //Ab hier fuer alle Strategien gleich
  if(useaddr != 0) {//gefunden, sonst return 0
    //Optimierung
    /*if(heap == extHeap) {
      if((os_getProcessSlot(os_getCurrentProc())->allocFrameEnd) < useaddr) {//neu allozierter Bereich befindet sich hinter allocFrameEnd
        os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = useaddr;
      }
      if((os_getProcessSlot(os_getCurrentProc())->allocFrameStart) > ((useaddr-size)+1)) {//neu allozierter Bereich befindet sich vor allocFrameStart
        os_getProcessSlot(os_getCurrentProc())->allocFrameStart = (useaddr-size)+1;
      }
    }*/
    if(heap->currentalloc == OS_MEM_NEXT) {//nur veraendern wenn Strategie NextFit verwendet wurde
      if(heap == intHeap) {
        nextaddrint = useaddr;//aktuallisiert gefundene letzte freie Speicheradresse fuer NextFit
      }
      if(heap == extHeap) {
        nextaddrext = useaddr;//aktuallisiert gefundene letzte freie Speicheradresse fuer NextFit
      }
    }
    for(uint16_t i=size; i>1; i--) {//i>1 weil der letzte Nibble mit ProzessID beschrieben werden muss
      os_setMapEntry(heap, useaddr, 0xF);//Beschreibt die Map-Eintraege mit F
      useaddr -= 1;
    }
    os_setMapEntry(heap, useaddr, os_getCurrentProc());//letztes bzw erstes Nibble mit ProzessID beschreiben

//aOptimierung------------------------------------------------------------------
    if(heap == extHeap) {
      //Fall 1: allokierter Bereich liegt zwischen bereits allokiertem
      //nichts tun
      //Fall 4: allokierter Bereich ist der erste im extHeap
      if(((os_getProcessSlot(os_getCurrentProc())->allocFrameStart) == os_getUseStart(heap)) && ((os_getProcessSlot(os_getCurrentProc())->allocFrameEnd) == os_getUseStart(heap))) {
        os_getProcessSlot(os_getCurrentProc())->allocFrameStart = useaddr;
        os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (useaddr+size)-1;
      }
      //Fall 2: allokierter Bereich liegt vor allen anderen
      if((os_getProcessSlot(os_getCurrentProc())->allocFrameStart) > useaddr) {
        os_getProcessSlot(os_getCurrentProc())->allocFrameStart = useaddr;
      }
      //Fall 3: allokierter Bereich liegt hinter allen anderen
      if((os_getProcessSlot(os_getCurrentProc())->allocFrameEnd) < useaddr) {
        os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (useaddr+size)-1;
      }
    }
//eOptimierung------------------------------------------------------------------

  }
  os_leaveCriticalSection();//Verlassen des kritischen Bereichs
  return useaddr;
}

/*MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size) {
  os_enterCriticalSection();
  MemAddr firstchunk = os_firstChunkAddress(heap, addr);
  MemAddr chunksize = os_getChunkSize(heap, addr);
  MemAddr usestart = os_getUseStart(heap);
  uint16_t usesize = os_getUseSize(heap);
  uint16_t framestart = os_getProcessSlot(os_getCurrentProc())->allocFrameStart;
  uint16_t frameend = os_getProcessSlot(os_getCurrentProc())->allocFrameEnd;
  if(os_getMapEntry(heap, addr)!= 0) {//sonst gibts nichts zum reallokieren
    if(os_getMapEntry(heap, firstchunk) == os_getCurrentProc()) {//Prozess darf nur seinen eigenen Speicher reallozieren
      uint16_t currentsize = chunksize;//prueft ob man vergroessern, verkleinern oder gleich bleiben will

//aVergroessern-----------------------------------------------------------------
      if(currentsize < size) {//vergroessern
        MemAddr start = firstchunk+chunksize;
        uint16_t difference = size - currentsize;//merkt sich wie viel man zusaetzlich allokieren muss
        bool found = true;

        //a1. Hinten ist komplett genug frei--------------------------------------
        for(uint16_t i=start; i<(start+difference); i++) {//sucht hinten nach genug Speicher
          if((i<(usestart+usesize)) && (os_getMapEntry(heap, i) == 0)) {//Use-Bereich ueberschritten oder nicht genug Platz frei

          }
          else {
            found = false;
            break;
          }
        }
        if(found == true) {//genug Speicher gefunden
          for(uint16_t i=start; i<(start+difference); i++) {//gefundenen Breich im Map-Bereich allozieren
            os_setMapEntry(heap, i, 0xF);
          }
          //aOptimierung---------------------------------------------
          if(heap == extHeap) {
            if((frameend) == (start-1)) {
              os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+difference)-1;
            }
          }
          //eOptimierung---------------------------------------------
          os_leaveCriticalSection();
          return firstchunk;
        }
      //e1. Hinten ist komplett genug frei--------------------------------------

      //a2. Vorne ist komplett genug frei---------------------------------------
      start = firstchunk-1;
      uint16_t counter = difference;//zaehlt ob komplett genug Speicher frei ist
      while((start>=usestart) && (os_getMapEntry(heap, start) == 0)) {//durchsucht Bereich vorne
        if(counter > 0) {
          counter -= 1;//zaehlt ob mindestens komplett genug Speicher frei ist
        }
        start -= 1;
      }
      start += 1;//Korrektur, jetzt ist start niedrigste freie Adresse
      if(counter == 0) {//vorne ist komplett genug frei
        //2.1 Merkt sich die Daten im aktuellen Speicher in einem Array
        MemValue values[currentsize];//Array speichert Daten im aktuell allozierten Speicherbereich
        uint16_t index = 0;
        for(uint16_t i=firstchunk; i<(firstchunk+chunksize); i++) {//durchlaeuft aktuell allozierten Speicherbereich
          values[index] = heap->driver->read(i);//schreibt Daten in das Array
          index += 1;
        }
        //2.2 alte Daten im Speicher loeschen
        //index = 0;
        //for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {//durchlaeuft aktuell allozierten Speicherbereich
          //heap->driver->write(i, 0);//schreibt Daten in das Array
        //}
        //2.3 Daten in den neuen Speicher schreiben
        index = 0;//reset
        for(uint16_t i=start; i<(start+size); i++) {//durchlaeuft neu allozierten Bereich
          if(index < currentsize) {//es gibt nur so viele Daten die reingeschrieben werden, die restlichen sind leer weil zusaetzlich alloziert
            heap->driver->write(i, values[index]);//schreibt gespeicherte Daten in den neuen Speicherbereich
          }
          index += 1;
        }
        //2.4 Map-Bereich des alten Bereichs freigeben
        MemAddr begin = firstchunk;
        MemAddr end = (firstchunk+currentsize);
        for(uint16_t i=begin; i<end; i++) {
          os_setMapEntry(heap, i, 0);
        }
        //2.5 Map-Bereich des neuen Bereichs besetzen
        os_setMapEntry(heap, start, os_getCurrentProc());
        for(uint16_t i=(start+1); i<(start+size); i++) {
          os_setMapEntry(heap, i, 0xF);
        }
        //aOptimierung---------------------------------------------
        if(heap == extHeap) {
          if((framestart) == begin) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = start;
          }
          if((frameend) == (end-1)) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+size)-1;
          }
        }
        //eOptimierung---------------------------------------------
        os_leaveCriticalSection();
        return start;
      }
      //e2. Vorne ist komplett genug frei---------------------------------------
//check
      //a3. Vorne nicht genug frei aber mit hinten genug frei-------------------
      found = true;
      start = firstchunk-1;
      counter = difference;//zaehlt ob komplett genug Speicher frei ist
      while((start>=usestart) && (os_getMapEntry(heap, start) == 0)) {//durchsucht Bereich vorne
        if(counter != 0) {
          counter -= 1;//zaehlt ob mindestens komplett genug Speicher frei ist
        }
        start -= 1;
      }
      start += 1;//Korrektur, jetzt ist start niedrigste freie Adresse
      //counter ist jetzt Groesse die noch hinten zu suchen ist, sollte nicht 0 sein sonst waere es in Fall 2 gelandet
      for(uint16_t i=(firstchunk+chunksize); i<(firstchunk+chunksize+counter); i++) {
        if((i<(usestart+usesize)) && (os_getMapEntry(heap, i) == 0)) {

        }
        else {
          found = false;
          break;
        }
      }
      if(found == true) {//genug Speicher vorne und hinten gefunden
        //3.1 Merkt sich die Daten im aktuellen Speicher in einem Array
        MemValue values[currentsize];//Array speichert Daten im aktuell allozierten Speicherbereich
        uint16_t index = 0;
        for(uint16_t i=firstchunk; i<(firstchunk+chunksize); i++) {//durchlaeuft aktuell allozierten Speicherbereich
          values[index] = heap->driver->read(i);//schreibt Daten in das Array
          index += 1;
        }
        //3.2 alte Daten im Speicher loeschen
        //index = 0;
        //for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {//durchlaeuft aktuell allozierten Speicherbereich
          //heap->driver->write(i, 0);//schreibt Daten in das Array
        //}
        //3.3 Daten in den neuen Speicher schreiben
        index = 0;//reset
        for(uint16_t i=start; i<(start+size); i++) {//durchlaeuft neu allozierten Bereich
          if(index < currentsize) {//es gibt nur so viele Daten die reingeschrieben werden, die restlichen sind leer weil zusaetzlich alloziert
            heap->driver->write(i, values[index]);//schreibt gespeicherte Daten in den neuen Speicherbereich
          }
          index += 1;
        }
        //3.4 Map-Bereich des alten Bereichs freigeben
        MemAddr begin = firstchunk;
        MemAddr end = firstchunk+currentsize;
        for(uint16_t i=begin; i<end; i++) {
          os_setMapEntry(heap, i, 0);
        }
        //3.5 Map-Bereich des neuen Bereichs besetzen
        os_setMapEntry(heap, start, os_getCurrentProc());
        for(uint16_t i=(start+1); i<(start+size); i++) {
          os_setMapEntry(heap, i, 0xF);
        }
        //aOptimierung---------------------------------------------
        if(heap == extHeap) {
          if((framestart) == begin) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = start;
          }
          if((frameend) == (end-1)) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+size)-1;
          }
        }
        //eOptimierung---------------------------------------------
        os_leaveCriticalSection();
        return start;
      }

      //4. Vorne und Hinten nicht genug Platz, komplett neu den Speicher durchsuchen, Optimierung durch os_malloc erledigt
      start = os_malloc(heap, size);
      if(start != 0) {
        //4.1 Merkt sich die Daten im aktuellen Speicher in einem Array
        MemValue values[currentsize];//Array speichert Daten im aktuell allozierten Speicherbereich
        uint16_t index = 0;
        for(uint16_t i=firstchunk; i<(firstchunk+chunksize); i++) {//durchlaeuft aktuell allozierten Speicherbereich
          values[index] = heap->driver->read(i);//schreibt Daten in das Array
          index += 1;
        }
        //4.3 Daten in den neuen Speicher schreiben
        index = 0;//reset
        for(uint16_t i=start; i<(start+size); i++) {//durchlaeuft neu allozierten Bereich
          if(index < currentsize) {//es gibt nur so viele Daten die reingeschrieben werden, die restlichen sind leer weil zusaetzlich alloziert
            heap->driver->write(i, values[index]);//schreibt gespeicherte Daten in den neuen Speicherbereich
          }
          index += 1;
        }
        //3.4 Map-Bereich des alten Bereichs freigeben
        MemAddr begin = firstchunk;
        MemAddr end = firstchunk+currentsize;
        for(uint16_t i=begin; i<end; i++) {
          os_setMapEntry(heap, i, 0);
        }
        //3.5 Map-Bereich des neuen Bereichs besetzen
        os_setMapEntry(heap, start, os_getCurrentProc());
        for(uint16_t i=(start+1); i<(start+size); i++) {
          os_setMapEntry(heap, i, 0xF);
        }
        //aOptimierung---------------------------------------------
        if(heap == extHeap) {
          if((framestart) == begin) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = start;
          }
          if((frameend) == (end-1)) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+size)-1;
          }
        }
        //eOptimierung---------------------------------------------
        os_leaveCriticalSection();
        return start;
      }
    }
//eVergroessern-----------------------------------------------------------------

//aVerkleinern------------------------------------------------------------------
    else if(currentsize > size) {//verkleinern
      for(uint16_t i=(os_firstChunkAddress(heap, addr)+size); i<(os_firstChunkAddress(heap, addr)+currentsize); i++) {//gibt den restlichen freizugegebenden Speicher frei
        os_setMapEntry(heap, i, 0);//Nibble aus 0 setzen und damit Speicherbereich freigeben
      }
      //aOptimierung---------------------------------------------
      if(heap == extHeap) {
        if(frameend == ((os_firstChunkAddress(heap, addr)+currentsize)-1)) {
          os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (os_firstChunkAddress(heap, addr)+size)-1;
        }
      }
      //eOptimierung---------------------------------------------
      os_leaveCriticalSection();
      return os_firstChunkAddress(heap, addr);
    }
//eVerkleinern------------------------------------------------------------------

//aGleichbleiebn----------------------------------------------------------------
    else {//gleich bleiben
      os_leaveCriticalSection();
      return os_firstChunkAddress(heap, addr);
    }
//eGleichbleiebn----------------------------------------------------------------

  }
  else {//Ungueltiger Reallokationsversuch
    os_error("Privatsphaere verletzt! :(");
  }
  os_leaveCriticalSection();
  return 0;
}
os_leaveCriticalSection();
return 0;
}*/
MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size) {
  os_enterCriticalSection();
  if((addr >= os_getUseStart(heap)) && (addr < (os_getUseStart(heap)+os_getUseSize(heap)))) {
    if(os_getMapEntry(heap, addr) != 0) {
      if(os_getMapEntry(heap, os_firstChunkAddress(heap, addr))==os_getCurrentProc()) {
      if(os_getChunkSize(heap, addr) < size) {
        MemAddr start = (os_firstChunkAddress(heap, addr) + os_getChunkSize(heap, addr));
        uint16_t counter = 0;
        uint16_t dif = (size - os_getChunkSize(heap, addr));
        MemAddr save = start;
        while((start<(os_getUseStart(heap)+os_getUseSize(heap))) && (os_getMapEntry(heap, start)==0)) {
          counter += 1;
          start+= 1;
        }
        start = save;
        if(counter >= dif) {//hinten genug gefunden
          for(uint16_t i=start; i<(start+dif); i++) {
            os_setMapEntry(heap, i, 0xF);
          }
          //Optimierung---------------------------------------------------------
          if(heap == extHeap) {
            if(os_getProcessSlot(os_getCurrentProc())->allocFrameEnd < (start+dif)) {
              os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+dif)-1;
            }
          }
          //Optimierung---------------------------------------------------------
          os_leaveCriticalSection();
          return os_firstChunkAddress(heap, addr);
        }
        else {//counter ist kleiner als dif
          start = (os_firstChunkAddress(heap, addr)-1);
          while((start>=os_getUseStart(heap)) && (os_getMapEntry(heap, start)==0)) {
            counter += 1;
            start -= 1;
          }
          start += 1;//weil Erhoehung nach Ueberpruefung, start ist jetzt niedrigste freie Adresse
          if(counter >= dif) {//hinten und vorne zusammen genug gefunden
            //1. Schritt: Daten des alten Bereichs sichern
            /*MemValue values[os_getChunkSize(heap, addr)];
            uint16_t index = 0;
            for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {
              values[index] = heap->driver->read(i);
              index += 1;
            }
            //2. Schritt: gespeicherte Daten in den neuen Bereich beschreiben
            index = 0;//reset
            for(uint16_t i=start; i<(start+size); i++) {
              heap->driver->write(i, values[index]);
              index += 1;
            }*/
            //1. Schritt: Daten vom alten in den neuen Bereich beschreiben
            MemAddr st=start;
            for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {
              heap->driver->write(start, (heap->driver->read(i)));
              start += 1;
            }
            start = st;
            //fuer Optimierung
            MemAddr first = os_firstChunkAddress(heap, addr);
            uint16_t chunksize = os_getChunkSize(heap, addr);
            //3. Schritt: Map des alten Bereichs freigeben
            for(uint16_t i=first; i<(first+chunksize); i++) {
              os_setMapEntry(heap, i, 0);
            }
            //4. Schritt: Map des neuen Bereichs besetzen
            os_setMapEntry(heap, start, os_getCurrentProc());
            for(uint16_t i=(start+1); i<(start+size); i++) {
              os_setMapEntry(heap, i, 0xF);
            }
            //Optimierung-------------------------------------------------------
            if(heap == extHeap) {
              if(os_getProcessSlot(os_getCurrentProc())->allocFrameEnd == ((first+chunksize)-1)) {//sonst stimmt was mit allocFrameEnd nicht
                os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+size)-1;
              }
              if(os_getProcessSlot(os_getCurrentProc())->allocFrameStart == first) {//sonst stimmt was mit allocFrameEnd nicht
                os_getProcessSlot(os_getCurrentProc())->allocFrameStart = start;
              }
            }
            //Optimierung-------------------------------------------------------
            os_leaveCriticalSection();
            return start;
          }
          else {//nur noch mit malloc alles neu durchsuchen
            start = os_malloc(heap, size);
            if(start != 0) {
              //1. Schritt: Daten des alten Bereichs sichern
              /*MemValue values[os_getChunkSize(heap, addr)];
              uint16_t index = 0;
              for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {
                values[index] = heap->driver->read(i);
                index += 1;
              }
              //2. Schritt: gespeicherte Daten in den neuen Bereich beschreiben
              index = 0;//reset
              for(uint16_t i=start; i<(start+size); i++) {
                heap->driver->write(i, values[index]);
                index += 1;
              }*/
              //1. Schritt: Daten vom alten in den neuen Bereich beschreiben
              MemAddr st=start;
              for(uint16_t i=os_firstChunkAddress(heap, addr); i<(os_firstChunkAddress(heap, addr)+os_getChunkSize(heap, addr)); i++) {
                heap->driver->write(start, (heap->driver->read(i)));
                start += 1;
              }
              start = st;
              //fuer Optimierung
              MemAddr first = os_firstChunkAddress(heap, addr);
              uint16_t chunksize = os_getChunkSize(heap, addr);
              //3. Schritt: Map des alten Bereichs freigeben
              for(uint16_t i=first; i<(first+chunksize); i++) {
                os_setMapEntry(heap, i, 0);
              }
              //4. Schritt: Map des neuen Bereichs besetzen
              os_setMapEntry(heap, start, os_getCurrentProc());
              for(uint16_t i=(start+1); i<(start+size); i++) {
                os_setMapEntry(heap, i, 0xF);
              }
              //Optimierung------------------------------------------------------- sollte in malloc schon passieren
              //if(os_getProcessSlot(os_getCurrentProc())->allocFrameEnd == ((first+chunksize)-1)) {//sonst stimmt was mit allocFrameEnd nicht
                //os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (start+size)-1;
              //}
              //if(os_getProcessSlot(os_getCurrentProc())->allocFrameStart == first) {//sonst stimmt was mit allocFrameEnd nicht
                //os_getProcessSlot(os_getCurrentProc())->allocFrameStart = start;
              //}
              //Optimierung-------------------------------------------------------
              os_leaveCriticalSection();
              return start;
            }
            else {//malloc hat auch nicht gefunden
              os_leaveCriticalSection();
              return 0;
            }
          }
        }
      }
      if(os_getChunkSize(heap, addr) > size) {//dann verkleinern
        MemAddr fs = os_firstChunkAddress(heap, addr);
        MemAddr sz = os_getChunkSize(heap, addr);
        for(uint16_t i=(fs+size); i<(fs+sz); i++) {
          os_setMapEntry(heap, i, 0);
        }
        os_leaveCriticalSection();
        return fs;
      }
      if(os_getChunkSize(heap, addr) == size) {
        os_leaveCriticalSection();
        return os_firstChunkAddress(heap, addr);
      }
    }
    }
  }
  else {
    os_error("Address false");
  }
  os_leaveCriticalSection();//nur fuer Funktion, man sollte hier nie ankommen
  return 0;
}

void os_free(Heap* heap, MemAddr addr) {
  os_enterCriticalSection();//Betreten des kritischen Bereichs
  if((addr < (os_getUseStart(heap) + os_getUseSize(heap))) && (addr >= os_getUseStart(heap))) {
  if(os_getMapEntry(heap, addr) != 0) {//sonst ist Speicherplatz frei und man muss nichts freigeben
    uint16_t size = os_getChunkSize(heap, addr);//merkt sich die Groesse des zu freigebenden Speichers
    MemAddr start = os_firstChunkAddress(heap, addr);//jetzt erst, sonst gibt es keine firstChunkAddress
    uint16_t limit = start+size;
    if(limit > (os_getUseStart(heap) + os_getUseSize(heap))) {
      limit = (os_getUseStart(heap) + os_getUseSize(heap));
    }
    if(os_getMapEntry(heap, start) > 7) {//Ungueltiger Freigabeversuch
      os_error("Please use os_sh_free!");
    }
    if((os_getMapEntry(heap, start) < 8) && (os_getMapEntry(heap, start) != os_getCurrentProc())) {//Ungueltiger Freigabeversuch
      os_error("Privatsphaere verletzt! :(");
    }
    if(os_getMapEntry(heap, start) == os_getCurrentProc()) {//Prozess darf nur seinen eigenen Speicher freigeben, im Nibble von start sollte die ProzessID stehen
      for(uint16_t i=start; i<limit; i++) {
        os_setMapEntry(heap, i, 0);
      }

//aOptimierung------------------------------------------------------------------
      if(heap == extHeap) {
        //4 Faelle betrachten
        //Fall 1: gefreeter Bereich liegt zwischen noch allozierten Bereich belegtem Bereich
        //In dem Fall muss nichts gemacht werden
        //Fall 2: gefreeter Bereich liegt vor allen anderen allozierten Bereichen
        //In dem Fall muss allocFrameStart veraendert werden
        if((os_getProcessSlot(os_getCurrentProc())->allocFrameStart) == start) {
          //druchsuche ab start den map Bereich nach ProzessID
          bool found = false;
          for(uint16_t j=start; j<(os_getUseStart(heap)+os_getUseSize(heap)); j++) {
            if(os_getMapEntry(heap, j) == os_getCurrentProc()) {
              os_getProcessSlot(os_getCurrentProc())->allocFrameStart = j;
              found = true;
              break;
            }
          }
          if(found == false) {//gerade gefreeter Chunk war der einzige im extHeap
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = os_getUseStart(heap);
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = os_getUseStart(heap);
          }
        }
        //Fall 3: gefreeter Bereich liegt hinter allen anderen allozierten Bereichen
        if((os_getProcessSlot(os_getCurrentProc())->allocFrameEnd) == ((start+size)-1)) {
          //durchsuche ab start+size-1 den map Bereich nach ProzessID (von unten nach oben)
          bool found = false;
          for(uint16_t k=(start+size)-1; k>=os_getUseStart(heap); k--) {
            if(os_getMapEntry(heap, k) == os_getCurrentProc()) {
              found = true;
              //neue End berechnen
              os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (k+os_getChunkSize(heap, k))-1;
              break;
            }
          }
          if(found == false) {//gerade gefreeter Chunk war der einzige im extHeap
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = os_getUseStart(heap);
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = os_getUseStart(heap);
          }
        }
      }
        //Fall 4: gefreeter Bereich war einziger Bereich im extHeap
        //wurde in Fall 2 und 3 schon betrachtet
        /*if((os_getProcessSlot(os_getCurrentProc())->allocFrameEnd) == (limit-1)) {//neu allozierter Bereich befindet sich hinter allocFrameEnd
          //durchsuchen
          MemAddr search = start;//irgendwo hinten beginnen
          while(os_getMapEntry(heap, search) != os_getCurrentProc()) {
            if(search < os_getUseStart(heap)) {
              break;
            }
            search -= 1;
          }
          //search hat jetzt letzten Chunk gefunden oder nicht
          if(search >= os_getUseStart(heap)) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = (search+os_getChunkSize(heap, search))-1;
          }
          else {//kein Chunk mehr im Speicher
            os_getProcessSlot(os_getCurrentProc())->allocFrameEnd = 0;
          }
        }
        if((os_getProcessSlot(os_getCurrentProc())->allocFrameStart) == start) {//neu allozierter Bereich befindet sich vor allocFrameStart
          //durchsuchen
          MemAddr search = start;
          while(os_getMapEntry(heap, search) != os_getCurrentProc()) {
            if(search > (os_getUseStart(heap)+os_getUseSize(heap))) {
              break;
            }
            search += 1;
          }
          //search hat jetzt ersten Chunk gefunden oder nicht
          if(search <= (os_getUseStart(heap)+os_getUseSize(heap))) {
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = search;
          }
          else {//kein Chunk mehr im Speicher
            os_getProcessSlot(os_getCurrentProc())->allocFrameStart = 0;
          }
        }*/
//aOptimierung------------------------------------------------------------------

    }
  }
  }
  os_leaveCriticalSection();//Verlassen des kritischen Bereichs
}

void os_freeProcessMemory(Heap* heap, ProcessID pid) {
  MemAddr start = os_getUseStart(heap);//Beginnt Suche bei der ersten Adresse im Use-Bereich um den Map-Bereich zu ueberpruefen
  MemAddr end = os_getUseStart(heap) + os_getUseSize(heap);

//aOptimierung------------------------------------------------------------------
  if(heap == extHeap) {
    start = os_getProcessSlot(pid)->allocFrameStart;//Beginnt Suche bei allocFrameStart wegen Optimierung
    end = (os_getProcessSlot(pid)->allocFrameEnd)+1;//wegen kleiner
  }
//eOptimierung------------------------------------------------------------------

  if(((heap == extHeap) && !((os_getProcessSlot(pid)->allocFrameStart==os_getUseStart(heap)) && (os_getProcessSlot(pid)->allocFrameEnd==os_getUseStart(heap)))) || heap == intHeap) {//sonst kein Chunk im extHeap, sollte man nicht brauchen
    bool find = false;//merkt sich ob pid gefunden, fuer nachfolgende Fs
    for (uint16_t i=start; i<end; i++) {
      if((find == true) && (os_getMapEntry(heap, i) == 0xF)) {
        os_setMapEntry(heap, i, 0);
      }
      else {
        find = false;
      }
      if(os_getMapEntry(heap, i) == pid) {
        os_setMapEntry(heap, i, 0);
        find = true;
      }
    }
  }
}

size_t os_getMapSize(Heap const* heap) {
  return heap->mapsize;
}

size_t os_getUseSize(Heap const* heap) {
  return heap->usesize;
}

MemAddr os_getMapStart(Heap const* heap) {
  return heap->mapstart;
}

MemAddr os_getUseStart(Heap const* heap) {
  return heap->usestart;
}

uint16_t os_getChunkSize(Heap const* heap, MemAddr addr) {
  os_enterCriticalSection();//Betreten des kritischen Bereichs
  uint16_t size = 0;
  if(os_getMapEntry(heap, addr) == 0) {//Speicherplatz ist frei und es gibt kein Chunk
    os_leaveCriticalSection();
    return size;
  }
  else {//Speicherbereich gehoert jemandem
    MemAddr start = os_firstChunkAddress(heap, addr);
    size = 1;//os_getMapEntry(heap, start) ist eine ProzessID
    while((os_getMapEntry(heap, start+1) == 0xF) && (start < (os_getUseStart(heap) + os_getUseSize(heap))-1)) {
      size += 1;
      start += 1;
    }
  }
  os_leaveCriticalSection();//Verlassen des kritischen Bereichs
  return size;
}

AllocStrategy os_getAllocationStrategy(Heap const* heap) {
  return heap->currentalloc;
}

void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat) {
  heap->currentalloc = allocStrat;
}

//Hilfsfunktionen
MemAddr os_firstChunkAddress(Heap const* heap, MemAddr addr) {//es wurde vor dem Aufruf schon ueberprueft, dass addr jemandem gehoert, also nicht leer ist
  while(os_getMapEntry(heap, addr) == 0xF) {
    addr -= 1;//fist Chunkt kann nur weiter vorne sein
  }
  return addr;
}

MemValue os_getMapEntry(Heap const* heap, MemAddr addr) {
  MemAddr place = addr - os_getUseStart(heap) + 1;//merkt sich die Stelle im Use-Bereich
  uint8_t mod = place % 2;//fuer High oder Low Nibble
  place = (os_getMapStart(heap) + (place / 2));//1 Byte im Mapbereich verwaltet 2 Byte im Nutzbereich
  if(mod == 1) {
    return os_getHighNibble(heap, place);
  }
  else {//mod == 0
    place -= 1;
    return os_getLowNibble(heap, place);
  }
}

void os_setMapEntry(Heap const* heap, MemAddr addr, MemValue value) {//selbe wie os_getMapEntry nur set nehmen
  MemAddr place = addr - os_getUseStart(heap) + 1;//merkt sich die Stelle im Use-Bereich
  uint8_t mod = place % 2;//fuer High oder Low Nibble
  place = (os_getMapStart(heap) + (place / 2));
  if(mod == 1) {
    os_setHighNibble(heap, place, value);
  }
  else {//mod == 0
    place -= 1;
    os_setLowNibble(heap, place, value);
  }
}

MemValue os_getHighNibble(Heap const* heap, MemAddr addr) {
  MemValue nibble = heap->driver->read(addr);
  return (nibble >> 4);
}

MemValue os_getLowNibble(Heap const* heap, MemAddr addr) {
  MemValue nibble = heap->driver->read(addr);
  return (nibble & 0b00001111);
}

void os_setHighNibble(Heap const* heap, MemAddr addr, MemValue value) {
  os_enterCriticalSection();
  MemValue nibble = heap->driver->read(addr);//Aktuelles Nibble rauslesen
  nibble &= 0b00001111;//reset des Highnibbles
  nibble |= (value << 4);//Setzen des neuen Nibbles
  heap->driver->write(addr, nibble);//Schreibt neues Nibble in Speichermedium
  os_leaveCriticalSection();
}

void os_setLowNibble(Heap const* heap, MemAddr addr, MemValue value) {
  os_enterCriticalSection();
  MemValue nibble = heap->driver->read(addr);//Aktuelles Nibble rauslesen
  nibble &= 0b11110000;//reset des Lownibbles
  nibble |= (value & 0b00001111);//Setzen des neuen Nibbles
  heap->driver->write(addr, nibble);//Schreibt neues Nibble in Speichermedium
  os_leaveCriticalSection();
}

//Gemeinsame Speicherbereiche---------------------------------------------------

/*
*Idee fuer Protokoll:
*Insgesamt: Zahlen von 0 bis 15
*Besetzt: Zahlen von 0 bis 7 und 15
*Frei: Zahlen von 8 bis 14
*Zahl 8: Speicherbereich geschlossen (kein Zugriff weder lesend noch schreibend)
*Zahl 9: Speicherbereich offen (schreibender Zugriff)
*Zahlen 10 bis 14: lesender Zugriff von maximal 5 Prozessen, 10 entspricht 1 lesender Prozess, 14 entspricht 5 lesende Prozesse
*/

MemAddr os_sh_malloc(Heap* heap, uint16_t size) {
  os_enterCriticalSection();//Betreten des kritischen Bereichs
  if(initializerint && (heap == intHeap)) {
    nextaddrint = os_getUseStart(heap)-1;
    initializerint = false;
  }
  if(initializerext && (heap == extHeap)) {
    nextaddrext = os_getUseStart(heap)-1;
    initializerext = false;
  }
  MemAddr useaddr = 0;//irgendwie initialisieren
  switch(heap->currentalloc) {
    case OS_MEM_FIRST: useaddr = os_memoryFirstFit(heap, size, os_getUseStart(heap)); break;
    case OS_MEM_NEXT:
    if(heap == intHeap) {
      useaddr = os_memoryNextFit(heap, size, nextaddrint+1);
    }
    if(heap == extHeap) {
      useaddr = os_memoryNextFit(heap, size, nextaddrext+1);
    }
    break;
    case OS_MEM_BEST: useaddr = os_memoryBestFit(heap, size); break;
    case OS_MEM_WORST: useaddr = os_memoryWorstFit(heap, size); break;
  }
  //Ab hier fuer alle Strategien gleich
  if(useaddr != 0) {//gefunden, sonst return 0
    if(heap->currentalloc == OS_MEM_NEXT) {//nur veraendern wenn Strategie NextFit verwendet wurde
      if(heap == intHeap) {
        nextaddrint = useaddr;//aktuallisiert gefundene letzte freie Speicheradresse fuer NextFit
      }
      if(heap == extHeap) {
        nextaddrext = useaddr;//aktuallisiert gefundene letzte freie Speicheradresse fuer NextFit
      }
    }
    for(uint16_t i=size; i>1; i--) {//i>1 weil der letzte Nibble mit ProzessID beschrieben werden muss
      os_setMapEntry(heap, useaddr, 0xF);//Beschreibt die Map-Eintraege mit F
      useaddr -= 1;
    }
    os_setMapEntry(heap, useaddr, 8);//letztes bzw erstes Nibble mit 8 (Speicherbereich geschlossen) beschreiben

//aOptimierung------------------------------------------------------------------
    //keine Optimierung noetig, weil Speicherbereich keinem Prozess alleine gehoert
//eOptimierung------------------------------------------------------------------

  }
  os_leaveCriticalSection();//Verlassen des kritischen Bereichs
  return useaddr;
}

void os_sh_free(Heap* heap, MemAddr* ptr) {
  os_enterCriticalSection();//Betreten des kritischen Bereichs
  if((*ptr < (os_getUseStart(heap) + os_getUseSize(heap))) && (*ptr >= os_getUseStart(heap))) {
  if(os_getMapEntry(heap, *ptr) != 0) {//sonst ist Speicherplatz frei und man muss nichts freigeben
    uint16_t size = os_getChunkSize(heap, *ptr);//merkt sich die Groesse des zu freigebenden Speichers
    //MemAddr start = os_firstChunkAddress(heap, *ptr);//jetzt erst, sonst gibt es keine firstChunkAddress
    uint16_t limit = os_firstChunkAddress(heap, *ptr)+size;
    if(limit > (os_getUseStart(heap) + os_getUseSize(heap))) {
      limit = (os_getUseStart(heap) + os_getUseSize(heap));
    }
    while(os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) > 8) {//warten bis Speicherbereich geschlossen ist
      os_yield();
    }
    if(os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) < 8) {//Ungueltgier Freigabeversuch
      os_error("Please use os_free!");
    }
    if(os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) == 8) {//Speicherbereich geschlossen und gemeinsamer Speicherbereich
      for(uint16_t i=os_firstChunkAddress(heap, *ptr); i<limit; i++) {
        os_setMapEntry(heap, i, 0);
      }

//aOptimierung------------------------------------------------------------------
        //keine Optimierung noetig, weil Speicherbereich keinem Prozess alleine gehoert
//aOptimierung------------------------------------------------------------------

    }
  }
  }
  os_leaveCriticalSection();//Verlassen des kritischen Bereichs
}

MemAddr os_sh_readOpen(Heap const* heap, MemAddr const *ptr) {
  os_enterCriticalSection();
  //MemAddr start = os_firstChunkAddress(heap, *ptr);//ueberall statt start ganz hinschreiben, sonst fuehrt dazu dass nur zwei Prozesse bei Test 4 angezeigt werden
  while((os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) == 9) || os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) == 14) {//warten bis schreibender Zugriff beendet und weniger als 5 Prozesse gerade lesen
    os_yield();//Rechenzeit abgeben
  }
  if(os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) == 8) {//Prozess ist einziger lesender Prozess
    os_setMapEntry(heap, os_firstChunkAddress(heap, *ptr), 10);
  }
  else {//es gibt mindestens einen Prozess der gerade liest
    os_setMapEntry(heap, os_firstChunkAddress(heap, *ptr), os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr))+1);
  }
  MemAddr addr = *ptr;//vor os_leaveCriticalSection dereferenzieren
  os_leaveCriticalSection();
  return addr;
}

MemAddr os_sh_writeOpen(Heap const* heap, MemAddr const *ptr) {
  os_enterCriticalSection();
  //MemAddr start = os_firstChunkAddress(heap, *ptr);
  while(os_getMapEntry(heap, os_firstChunkAddress(heap, *ptr)) > 8) {//warten bis niemand mehr schreibt und niemand mehr liest
    os_yield();//Rechenzeit abgeben
  }
  os_setMapEntry(heap, os_firstChunkAddress(heap, *ptr), 9);//schreibender Zugriff (Speicherbereich oeffnen)
  MemAddr addr = *ptr;//vor os_leaveCriticalSection dereferenzieren
  os_leaveCriticalSection();
  return addr;
}

void os_sh_close(Heap const* heap, MemAddr addr) {
  os_enterCriticalSection();
  //MemAddr start = os_firstChunkAddress(heap, addr);
  if((os_getMapEntry(heap, os_firstChunkAddress(heap, addr)) == 9) || (os_getMapEntry(heap, os_firstChunkAddress(heap, addr)) == 10) ) {//schreibender Zugriff oder nur einziger lesenden Zugriff schliessen
    os_setMapEntry(heap, os_firstChunkAddress(heap, addr), 8);//Speicherbereich geschlossen
  }
  if(os_getMapEntry(heap, os_firstChunkAddress(heap, addr)) > 10) {//momentan mehr als einen lesenden Zugriff
    os_setMapEntry(heap, os_firstChunkAddress(heap, addr), os_getMapEntry(heap, os_firstChunkAddress(heap, addr))-1);//lesende Prozesse um eins reduzieren
  }
  os_leaveCriticalSection();
}

void os_sh_write(Heap const* heap, MemAddr const* ptr, uint16_t offset, MemValue const* dataSrc, uint16_t length) {
  MemAddr pointer = os_sh_writeOpen(heap, ptr);
  //MemAddr start = os_firstChunkAddress(heap, pointer);
  if((length+offset) <= os_getChunkSize(heap, pointer)) {
    for(uint16_t i=0; i<length; i++) {//length Bytes in den Bereich den gemeinsamen Speicherbereich schreiben
      heap->driver->write(i+offset+os_firstChunkAddress(heap, pointer), intHeap->driver->read(i+((MemAddr)dataSrc)));//Kein Array!!!
    }
  }
  else {
    os_error("Cant write beyond chunk!");//Fehlermeldung
  }
  os_sh_close(heap, pointer);
}

void os_sh_read(Heap const* heap, MemAddr const* ptr, uint16_t offset, MemValue* dataDest, uint16_t length) {
  MemAddr pointer = os_sh_readOpen(heap, ptr);//Bereich fuer lesenden Zugriff vorbereiten
  //MemAddr start = os_firstChunkAddress(heap, pointer);
  if((length+offset) <= os_getChunkSize(heap, pointer)) {
    for(uint16_t i=0; i<length; i++) {//length Bytes in den Bereich dataDest kopieren
      intHeap->driver->write(i+((MemAddr)dataDest), heap->driver->read(i+offset+os_firstChunkAddress(heap, pointer)));//Kein Array!!!
    }
  }
  else {
    os_error("Cant read beyond chunk!");//Fehlermeldung
  }
  os_sh_close(heap, pointer);
}
