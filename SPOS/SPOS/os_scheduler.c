#include "os_scheduler.h"
#include "util.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "os_core.h"
#include "lcd.h"
#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"
#include "os_memory.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals
bool notask = true;
//----------------------------------------------------------------------------

//! Array of states for every possible process
//#warning IMPLEMENT STH. HERE
Process os_processes[MAX_NUMBER_OF_PROCESSES];

//! Array of function pointers for every registered program
//#warning IMPLEMENT STH. HERE
Program* os_programs[MAX_NUMBER_OF_PROGRAMS];

//! Index of process that is currently executed (default: idle)
//#warning IMPLEMENT STH. HERE
ProcessID currentProc;

//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
//#warning IMPLEMENT STH. HERE
SchedulingStrategy currentstrat;

//! Count of currently nested critical sections
//#warning IMPLEMENT STH. HERE
uint8_t criticalSectionCount;

//! Used to auto-execute programs.
uint16_t os_autostart;

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect) __attribute__((naked));

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

void restoreBlockedProcesses(void){
	for(uint8_t pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++){
		if(os_processes[pid].state == OS_PS_BLOCKED){
			os_processes[pid].state = OS_PS_READY;
		}
	}
}
/*!
 *  Timer interrupt that implements our scheduler. Execution of the running
 *  process is suspended and the context saved to the stack. Then the periphery
 *  is scanned for any input events. If everything is in order, the next process
 *  for execution is derived with an exchangeable strategy. Finally the
 *  scheduler restores the next process for execution and releases control over
 *  the processor to that process.
 */
ISR(TIMER2_COMPA_vect) {
    //1. Geschieht implizit
	
    //2. Sichern des Laufzeitkontextes des aktuellen Prozesses auf dessen Prozessstack
    saveContext();
	
    //3. Sichern des Stackpointers fuer den Prozessstack des aktuellen Prozesses
    os_processes[os_getCurrentProc()].sp.as_int = SP;
	
    //Ermittelt und speichert den Hashwert
    os_processes[os_getCurrentProc()].checksum = os_getStackChecksum(os_getCurrentProc());
	
    //4. Setzen des SP-Registers auf den Scheduler-Stack
    SP = BOTTOM_OF_ISR_STACK;
	
    //Aufruf des Taskmanagers
    if(((os_getInput() & 0b00001001) == 0b00001001) && notask) {//Auf druecken von Enter+ESC warten
      os_waitForNoInput();//Warten bis Buttons losgelassen wurden
      os_taskManMain();//Aufruf des Taskmanagers
    }
	
    //5. Setzen des Prozesszustandes des aktuellen Prozesses auf OS_PS_READY
    if((os_processes[os_getCurrentProc()].state != OS_PS_UNUSED) && (os_processes[os_getCurrentProc()].state != OS_PS_BLOCKED)) {//wichtig fuer os_kill
      os_processes[os_getCurrentProc()].state = OS_PS_READY;
    }
	
    //6. Auswahl des naechsten fortzusetzenden Prozesszustandes
    switch(os_getSchedulingStrategy()) {
      case OS_SS_EVEN: currentProc = os_Scheduler_Even(os_processes, os_getCurrentProc()); break;
      case OS_SS_RANDOM: currentProc = os_Scheduler_Random(os_processes, os_getCurrentProc()); break;
      case OS_SS_RUN_TO_COMPLETION: currentProc = os_Scheduler_RunToCompletion(os_processes, os_getCurrentProc()); break;
      case OS_SS_ROUND_ROBIN: currentProc = os_Scheduler_RoundRobin(os_processes, os_getCurrentProc()); break;
      case OS_SS_INACTIVE_AGING: currentProc = os_Scheduler_InactiveAging(os_processes, os_getCurrentProc()); break;
	  case OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE: currentProc = os_Scheduler_MLFQ(os_processes, os_getCurrentProc()); break;
    }
	
	// Hier sollen Prozesse, die blocked ignoriert wurden, wieder zu OS_PS_READY gesetzt werden
	restoreBlockedProcesses();
	
	// Check the checksum
    if(os_processes[os_getCurrentProc()].checksum != os_getStackChecksum(os_getCurrentProc())) {
      notask = false;
      os_error("Hashwert nicht gleich -_-");
    }
	
	// Task Manager call
    if(((os_getInput() & 0b00001001) == 0b00001001) && !notask) {//Auf druecken von Enter+ESC warten
      os_waitForNoInput();//Warten bis Buttons losgelassen wurden
      lcd_clear();
      TIMSK2 &= 0b11111101;//Deaktivieren des Schedulers
    }
	
    //7. Setzen des Prozesszustandes des fortzusetzenden Prozesses auf OS_PS_RUNNING
    os_processes[os_getCurrentProc()].state = OS_PS_RUNNING;
	
    //8. Wiederherstellen des Stackpointers fuer den Prozessstack des fortzusetzenden Prozesses
    SP = os_processes[os_getCurrentProc()].sp.as_int;
	
    //9. Wiederherstellen des Laufzeitkontestes des fortzusetzenden Prozesses
    restoreContext();
	
    //10. Automatischer Ruecksprung
}

/*!
 *  Used to register a function as program. On success the program is written to
 *  the first free slot within the os_programs array (if the program is not yet
 *  registered) and the index is returned. On failure, INVALID_PROGRAM is returned.
 *  Note, that this function is not used to register the idle program.
 *
 *  \param program The function you want to register.
 *  \return The index of the newly registered program.
 */
ProgramID os_registerProgram(Program* program) {
    ProgramID slot = 0;

    // Find first free place to put our program
    while (os_programs[slot] &&
           os_programs[slot] != program && slot < MAX_NUMBER_OF_PROGRAMS) {
        slot++;
    }

    if (slot >= MAX_NUMBER_OF_PROGRAMS) {
        return INVALID_PROGRAM;
    }

    os_programs[slot] = program;
    return slot;
}

/*!
 *  Used to check whether a certain program ID is to be automatically executed at
 *  system start.
 *
 *  \param programID The program to be checked.
 *  \return True if the program with the specified ID is to be auto started.
 */
bool os_checkAutostartProgram(ProgramID programID) {
    return !!(os_autostart & (1 << programID));
}

/*!
 *  This is the idle program. The idle process owns all the memory
 *  and processor time no other process wants to have.
 */
PROGRAM(0, AUTOSTART) {
    //#warning IMPLEMENT STH. HERE
    while(1) {//Endlosschleife
      lcd_writeChar('.');
      delayMs(DEFAULT_OUTPUT_DELAY);
    }
}

/*!
 * Lookup the main function of a program with id "programID".
 *
 * \param programID The id of the program to be looked up.
 * \return The pointer to the according function, or NULL if programID is invalid.
 */
Program* os_lookupProgramFunction(ProgramID programID) {
    // Return NULL if the index is out of range
    if (programID >= MAX_NUMBER_OF_PROGRAMS) {
        return NULL;
    }

    return os_programs[programID];
}

/*!
 * Lookup the id of a program.
 *
 * \param program The function of the program you want to look up.
 * \return The id to the according slot, or INVALID_PROGRAM if program is invalid.
 */
ProgramID os_lookupProgramID(Program* program) {
    ProgramID i;

    // Search program array for a match
    for (i = 0; i < MAX_NUMBER_OF_PROGRAMS; i++) {
        if (os_programs[i] == program) {
            return i;
        }
    }

    // If no match was found return INVALID_PROGRAM
    return INVALID_PROGRAM;
}

/*!
 *  This function is used to execute a program that has been introduced with
 *  os_registerProgram.
 *  A stack will be provided if the process limit has not yet been reached.
 *  In case of an error, an according message will be displayed on the LCD.
 *  This function is multitasking safe. That means that programs can repost
 *  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
 *
 *  \param programID The program id of the program to start (index of os_programs).
 *  \param priority A priority ranging 0..255 for the new process:
 *                   - 0 means least favorable
 *                   - 255 means most favorable
 *                  Note that the priority may be ignored by certain scheduling
 *                  strategies.
 *  \return The index of the new process (throws error on failure and returns
 *          INVALID_PROCESS as specified in defines.h).
 */
ProcessID os_exec(ProgramID programID, Priority priority) {
    //#warning IMPLEMENT STH. HERE
    os_enterCriticalSection();//Betreten des kritischen Bereichs
    //1. Freien Platz im Array os_processes finden
    uint8_t freeplace = 0;//merkt sich den ersten freien Platz im Array
    for(uint8_t i=0; i<MAX_NUMBER_OF_PROCESSES;i++) {//durchlaeuft das Array
      if((os_processes[i].state) == OS_PS_UNUSED) {//Platz gefunden
        break;
      }
      freeplace = (i+1);
    }
    if(freeplace >= MAX_NUMBER_OF_PROCESSES) {//alle Plaetze belegt
      os_leaveCriticalSection();
      return INVALID_PROCESS;
    }
    //2. Funktionszeiger aus dem Array os_programs laden
    Program* functionpointer = os_lookupProgramFunction(programID);
    if(functionpointer == NULL) {//Abbruch wegen Fehlerfall
      os_leaveCriticalSection();
      return INVALID_PROCESS;
    }
    //3. Programmindex, Prozesszustand und Prozessprioritaet speichern
    os_processes[freeplace].state = OS_PS_READY;//Prozesszustand mit READY initialisieren
    os_processes[freeplace].progID = programID;//Programmindex speichern
    os_processes[freeplace].priority = priority;//Prozessprioritaet speichern
    //Optimierung
    os_processes[freeplace].allocFrameStart = os_getUseStart(extHeap);
    os_processes[freeplace].allocFrameEnd = os_getUseStart(extHeap);
    os_resetProcessSchedulingInformation(freeplace);//Alter des Prozesses auf 0 setzen
    //4. Prozessstack vorbereiten
    //StackPointer stackpointer;
    os_processes[freeplace].sp.as_int = PROCESS_STACK_BOTTOM(freeplace);//Anfang des Stacks (ganz unten)
    //if((PROCESS_STACK_BOTTOM(freeplace) - stackpointer) <= STACK_SIZE_PROC) {//pruefen ob genug Platz
      *(os_processes[freeplace].sp.as_ptr) = (uint8_t)((uint16_t)&os_dispatcher);//Programmzaehler (Low-Byte) speichern
      os_processes[freeplace].sp.as_int -= 1;//Zeiger aktualisieren
    //}
    //else {
      //os_error("Stack too small! :(");
    //}
    //if((PROCESS_STACK_BOTTOM(freeplace) - stackpointer) <= STACK_SIZE_PROC) {//pruefen ob genug Platz
      *(os_processes[freeplace].sp.as_ptr) = (uint8_t)(((uint16_t)&os_dispatcher) >> 8);//Programmzaehler (High-Byte) speichern
      os_processes[freeplace].sp.as_int -= 1;//Zeiger aktualisieren
    //}
    //else {
      //os_error("Stack too small! :(");
    //}
    for(uint8_t j=0; j<33; j++) {
      //if((PROCESS_STACK_BOTTOM(freeplace) - stackpointer) <= STACK_SIZE_PROC) {//pruefen ob genug Platz
        *(os_processes[freeplace].sp.as_ptr) = 0;//Null-Bytes
        os_processes[freeplace].sp.as_int -= 1;//stackpointer verschieben,zeigt am Ende auf die erste freie Speicherstelle
      //}
      //else {
        //os_error("Stack too small! :(");
      //}
    }
    os_processes[freeplace].checksum = os_getStackChecksum(freeplace);//StackChecksum korrekt initialisieren
    os_leaveCriticalSection();//Verlassen des kritischen Bereichs
    return freeplace;
}

/*!
 *  If all processes have been registered for execution, the OS calls this
 *  function to start the idle program and the concurrent execution of the
 *  applications.
 */
void os_startScheduler(void) {
    //#warning IMPLEMENT STH. HERE
    extern uint8_t const __heap_start;//Erste freie Speicherstelle des Heaps
        if((uint16_t)&__heap_start > (intSRAM->start + SAVE_DISTANCE)) {
          os_error("Save distance too small! :(");
        }
    currentProc = 0;//Leerlaufprozess starten
    os_processes[0].state = OS_PS_RUNNING;//Zustand des Leerlaufprozesses auf OS_PS_RUNNING setzen
    SP = os_processes[0].sp.as_int;//Setzen des Stackpointers auf den Prozessstack des Leerlaufprozesses
    restoreContext();//Sprung in den Leerlaufprozess
}

/*!
 *  In order for the Scheduler to work properly, it must have the chance to
 *  initialize its internal data-structures and register.
 */
void os_initScheduler(void) {
    //#warning IMPLEMENT STH. HERE
    for(uint8_t i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {//Array os_processes vorbereiten
      os_processes[i].state = OS_PS_UNUSED;
    }
    for(uint8_t j=0; j<MAX_NUMBER_OF_PROGRAMS; j++) {
      if(os_checkAutostartProgram(j)){//fragt, ob Programm automatisch gestartet werden soll
        os_exec(j, DEFAULT_PRIORITY);
      }
    }
}

/*!
 *  A simple getter for the slot of a specific process.
 *
 *  \param pid The processID of the process to be handled
 *  \return A pointer to the memory of the process at position pid in the os_processes array.
 */
Process* os_getProcessSlot(ProcessID pid) {
    return os_processes + pid;
}

/*!
 *  A simple getter for the slot of a specific program.
 *
 *  \param programID The ProgramID of the process to be handled
 *  \return A pointer to the function pointer of the program at position programID in the os_programs array.
 */
Program** os_getProgramSlot(ProgramID programID) {
    return os_programs + programID;
}

/*!
 *  A simple getter to retrieve the currently active process.
 *
 *  \return The process id of the currently active process.
 */
ProcessID os_getCurrentProc(void) {
    //#warning IMPLEMENT STH. HERE
    return currentProc;
}

/*!
 *  This function return the the number of currently active process-slots.
 *
 *  \returns The number currently active (not unused) process-slots.
 */
uint8_t os_getNumberOfActiveProcs(void) {
    uint8_t num = 0;

    ProcessID i = 0;
    do {
        num += os_getProcessSlot(i)->state != OS_PS_UNUSED;
    } while (++i < MAX_NUMBER_OF_PROCESSES);

    return num;
}

/*!
 *  This function returns the number of currently registered programs.
 *
 *  \returns The amount of currently registered programs.
 */
uint8_t os_getNumberOfRegisteredPrograms(void) {
    uint8_t i;
    for (i = 0; i < MAX_NUMBER_OF_PROGRAMS && *(os_getProgramSlot(i)); i++);
    // Note that this only works because programs cannot be unregistered.
    return i;
}

/*!
 *  Sets the current scheduling strategy.
 *
 *  \param strategy The strategy that will be used after the function finishes.
 */
void os_setSchedulingStrategy(SchedulingStrategy strategy) {
    //#warning IMPLEMENT STH. HERE
    os_resetSchedulingInformation(strategy);//Vorbereitung fuer RoundRobin und InactiveAging
    currentstrat = strategy;
}

/*!
 *  This is a getter for retrieving the current scheduling strategy.
 *
 *  \return The current scheduling strategy.
 */
SchedulingStrategy os_getSchedulingStrategy(void) {
    //#warning IMPLEMENT STH. HERE
    return currentstrat;
}

/*!
 *  Enters a critical code section by disabling the scheduler if needed.
 *  This function stores the nesting depth of critical sections of the current
 *  process (e.g. if a function with a critical section is called from another
 *  critical section) to ensure correct behavior when leaving the section.
 *  This function supports up to 255 nested critical sections.
 */
void os_enterCriticalSection(void) {
    //#warning IMPLEMENT STH. HERE
    uint8_t gieb = (SREG >> 7);//Speichern des GIEB
    SREG &= 0b01111111;//Deaktivieren des GIEB
    criticalSectionCount += 1;//Inkrementieren der Verschachtelungstiefe
    TIMSK2 &= 0b11111101;//Deaktivieren des Schedulers
    SREG |= (gieb << 7);//Wiederherstellen des GIEB im SREG
}

/*!
 *  Leaves a critical code section by enabling the scheduler if needed.
 *  This function utilizes the nesting depth of critical sections
 *  stored by os_enterCriticalSection to check if the scheduler
 *  has to be reactivated.
 */
void os_leaveCriticalSection(void) {
    //#warning IMPLEMENT STH. HERE
    uint8_t gieb = (SREG >> 7);//Speichern des GIEB
    SREG &= 0b01111111;//Deaktivieren des GIEB
    criticalSectionCount -= 1;
    if(criticalSectionCount == 0) {//prueft ob os_enterCriticalSection genauso oft aufgerufen wurde wie os_leaveCriticalSection
      TIMSK2 |= 0b00000010;//Aktivieren des Schedulers
    }
    if(criticalSectionCount < 0) {
      os_error("too much leave critical section");
    }
    SREG |= (gieb << 7);//Wiederherstellen des GIEB im SREG
}

/*!
 *  Calculates the checksum of the stack for a certain process.
 *
 *  \param pid The ID of the process for which the stack's checksum has to be calculated.
 *  \return The checksum of the pid'th stack.
 */
StackChecksum os_getStackChecksum(ProcessID pid) {
    //#warning IMPLEMENT STH. HERE
    uint8_t checksum = 0;//Variable fuer Ergebnis
    for(uint16_t i=PROCESS_STACK_BOTTOM(pid); i>os_processes[pid].sp.as_int; i--) {//laeuft vom Boden des Stacks (groesste Adresse) bis zum jetzigen Stackpointer
      checksum ^= (*(uint8_t*)i);
    }
    return checksum;
}

bool os_kill(ProcessID pid) {//muss ueber os_dispatcher stehen
  os_enterCriticalSection();//wichtig fuer Freigabe
  if((pid == 0) || (pid > 7)) {//Idle-Prozess darf nicht terminieren, gueltige pid nur 1-7
    os_leaveCriticalSection();
    return false;
  }
  os_processes[pid].state = OS_PS_UNUSED;//Prozess-Array-Stelle aufraeumen/freigeben
  os_freeProcessMemory(intHeap, pid);//loescht alle allozierten Speicherbereiche des aktuellen Prozesses falls dieser terminiert ist
  os_freeProcessMemory(extHeap, pid);
  if(pid == os_getCurrentProc()) {//Prozess terminiert sich selbst
    criticalSectionCount = 1;//da in os_leaveCriticalSection() dann -1 ausgefuehrt, also 0 wird
    os_leaveCriticalSection();
    while(os_getCurrentProc() == pid) {//Scheduler sollte jetzt aktiv sein und warten bis naechster Prozess ausgewaehlt wurde
    }
    return true;
  }
  os_leaveCriticalSection();
  return true;
}

void os_dispatcher(void) {
  ProcessID i = os_getCurrentProc();//ProcessID merken
  Program* j = os_lookupProgramFunction(os_processes[i].progID);//Programmfunktion merken
  j();//direkter Aufruf der Programmfunktion mittels Funktionszeiger
  //os_processes[i].state = OS_PS_UNUSED;//Prozess terminiert, erfolgt bereits in os_kill
  os_kill(i);//Terminierungsverwaltung
  while(1) {//Endlosschleife, wartet bis naechster Prozess von Scheduler ausgewaehlt wird
  }
}

void os_yield(void){
	os_enterCriticalSection();
	os_processes[os_getCurrentProc()].state = OS_PS_BLOCKED; // So that the process won't be chosen by the scheduler
	
	// Save everything before temporarily leaving 
	uint8_t GIEB = SREG >> 7; // Save the GIEB
	uint8_t local_CS_count = criticalSectionCount; // Save local CS count
	
	// We quasi leave the critical section
	criticalSectionCount = 0; // In order not to mess up with critical sections of other processes
	SREG |= 0b10000000; // Turn on GIEB for other processes 
	TIMSK2 |= 0b00000010; // Activate the scheduler
	
	
	TIMER2_COMPA_vect(); // Manually call the scheduler so other processes may have processing time
	
	/*---------------------------------------------------
	This is where the scheduler chooses this process again.
	We are now back to os_yield, thus everything must be 
	restored like before we left.
	-----------------------------------------------------*/
	
	// We quasi re-enter the critical section
	TIMSK2 &= 0b11111101; // Deactivate the scheduler
	criticalSectionCount = local_CS_count; // Restore the local CS count
	if(GIEB == 0){ // Restore the state of GIEB
		SREG &= 0b01111111;
	} else {
		SREG |= 0b10000000;
	} 
	os_leaveCriticalSection();
}
