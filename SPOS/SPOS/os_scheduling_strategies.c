#include "os_scheduling_strategies.h"
#include "defines.h"
#include "os_core.h"
#include <string.h>

#include <stdlib.h>

//Global variables
SchedulingInformation schedulingInfo;

ProcessQueue pqueue[NUMBER_OF_QUEUES]; // The P-Queues with 0 being the highest priority

uint8_t MLFQ_time[MAX_NUMBER_OF_PROCESSES]; // Array to save MLFQ time slice of all processes

ProcessQueue* MLFQ_getQueue(uint8_t queueID);

uint8_t MLFQ_getDefaultTimeslice(uint8_t queueID);

uint8_t MLFQ_MapToQueue (Priority prio);

void MLFQ_initProcessTime(ProcessID pid){ // Newly spawned process
	if(pid != 0){ // Must be not idle
		uint8_t queue_id = MLFQ_MapToQueue(os_getProcessSlot(pid)->priority);
		MLFQ_time[pid] = MLFQ_getDefaultTimeslice(queue_id);
	}
}

// init MLFQ only
void os_initSchedulingInformation(void){ // This is called in os_initScheduler
	for(uint8_t i = 0; i < NUMBER_OF_QUEUES; i++){
		pqueue_init(&pqueue[i]);
	}
}

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 * \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(SchedulingStrategy strategy) { // This is called in os_setSchedulingStrategy
    // This is a presence task
    if(strategy == OS_SS_ROUND_ROBIN) {
      schedulingInfo.timeSlice = os_getProcessSlot(os_getCurrentProc())->priority;//Setze Zeitscheibe auf Prioritaet des entsprechenden Prozesses
    }
    if(strategy == OS_SS_INACTIVE_AGING) {
      for(uint8_t i=1; i<MAX_NUMBER_OF_PROCESSES; i++) {//Alter aller Prozesse auf 0 setzen
        schedulingInfo.age[i] = 0;
      }
    }
	if(strategy == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE){ 
		for(uint8_t queueID = 0; queueID < NUMBER_OF_QUEUES; queueID++){ // First: Reset the queues
			pqueue_reset(&pqueue[queueID]);
		}
		for(uint8_t pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++){ // Second: Insert all active processes in the queues
			if(os_getProcessSlot(pid)->state != OS_PS_UNUSED){
				MLFQ_initProcessTime(pid);
				pqueue_append(&pqueue[MLFQ_MapToQueue(os_getProcessSlot(pid)->priority)], pid);
			}
		}
	}
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID pid) { // This is called in os_exec
    if(os_getSchedulingStrategy() == OS_SS_INACTIVE_AGING){
		schedulingInfo.age[pid] = 0;
	}
	if(os_getSchedulingStrategy() == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE){
		MLFQ_initProcessTime(pid);
		pqueue_append(&pqueue[MLFQ_MapToQueue(os_getProcessSlot(pid)->priority)], pid);
	}
}

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(Process const processes[], ProcessID current) {
    //#warning IMPLEMENT STH. HERE
    for(uint8_t i=(current+1); i<MAX_NUMBER_OF_PROCESSES; i++) {//Durchlaeuft das Array ab rechts von current bis zum Ende des Arrays
      if(processes[i].state == OS_PS_READY) {//Sucht nach Prozess der READY ist
        return i;
      }
    }
    for(uint8_t j=1; j<(current+1); j++) {//Durchlaeuft das Array vom Anfang bis links von current falls in der ersten Schleife nichts gefunden wurde
      if(processes[j].state == OS_PS_READY) {//Sucht nach Prozess der READY ist
        return j;
      }
    }
    return 0;//kein Prozess READY, gehe in Leerlaufprozess
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(Process const processes[], ProcessID current) {
    //#warning IMPLEMENT STH. HERE
    uint8_t chosen = 0;
    uint8_t helpindex = 0;//Hilfsvariable die help der Reihe nach mit Prozessen fuellt
    uint8_t help[MAX_NUMBER_OF_PROCESSES];//Hilfsarray welches alle Prozessindizes speichert, wo der Prozess READY ist

    for(uint8_t i=1; i<MAX_NUMBER_OF_PROCESSES; i++) {//auf Beginn bei 1 achten!!!
      if(processes[i].state == OS_PS_READY) {
        help[helpindex] = i;//speichert Index der READY-Prozesse
        helpindex += 1;
      }
    }
    if(helpindex == 0) {//kein Prozess READY, gehe in Leerlaufprozess
      return 0;
    }
    else {
      chosen = (rand() % helpindex);//setzt Zufallszahl in den gewuenschten Zahlenbereich
      return help[chosen];//gibt Index des ausgewaehlten Prozesses in processes[] wieder
    }
}


/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(Process const processes[], ProcessID current) {
    // This is a presence task
    if((schedulingInfo.timeSlice > 1) && processes[current].state == OS_PS_READY) {
      schedulingInfo.timeSlice -= 1;
      return current;
    }
    else {//Zeitscheibe abgelaufen
      ProcessID new = os_Scheduler_Even(processes, current);
      if(new != 0) {//nicht der Leerlaufprozess
        schedulingInfo.timeSlice = processes[new].priority;//Setze Zeitscheibe auf Prioritaet des entsprechenden Prozesses
        return new;
      }
      else {//kein Prozess READY
        return 0;
      }
    }
}

/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to its priority.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(Process const processes[], ProcessID current) {
    // This is a presence task
    //1. Alter aller inaktiven Prozesse wird um deren Prioritaet erhoeht
    for(uint8_t i=1; i<MAX_NUMBER_OF_PROCESSES; i++) {
      if((processes[i].state == OS_PS_READY) && (i != current)) {
        schedulingInfo.age[i] += processes[i].priority;
      }
    }
    //2. Hoechstes Alter wird gesucht
    uint8_t oldest = 0;//merkt sich das hoechste Alter
    for(uint8_t j=1; j<MAX_NUMBER_OF_PROCESSES; j++) {
      if((schedulingInfo.age[j] > oldest) && (processes[j].state == OS_PS_READY)) {
        oldest = schedulingInfo.age[j];
      }
    }
    //3. Alle Prozesse mit oldest als Alter merken
    uint8_t sameage[MAX_NUMBER_OF_PROCESSES];
    uint8_t sameageindex = 0;
    for(uint8_t k=1; k<MAX_NUMBER_OF_PROCESSES; k++) {//schreibt alle Prozesse mit oldest Alter ins Array
      if((schedulingInfo.age[k] == oldest) && (processes[k].state == OS_PS_READY)) {
        sameage[sameageindex] = k;
        sameageindex += 1;
      }
    }
    if(sameageindex == 1) {//Nur ein Prozess hat Alter oldest
      schedulingInfo.age[sameage[0]] = processes[sameage[0]].priority;//Alter des ausgewaehlten Prozesses auf Prioritaet zuruecksetzen
      return sameage[0];
    }
    //4. Sucht Prozess mit hoechster Prioritaet
    if(sameageindex > 1){//mehr als ein Prozess haben das gleiche hoechste Alter
      uint8_t highestprio = 0;
      for(uint8_t l=0; l<sameageindex; l++) {//sucht die groesste Prioritaet
        if(processes[sameage[l]].priority > highestprio) {
          highestprio = processes[sameage[l]].priority;
        }
      }
      //Alle Prozesse mit hoechster Prioritaet merken
      uint8_t sameprio[sameageindex];//merkt sich alle Prozesse mit gleicher Prioritaet
      uint8_t sameprioindex = 0;
      for(uint8_t m=0; m<sameageindex; m++) {//schreibt alle Prozesse mit hoechster Prioritaet ins Array
        if(processes[sameage[m]].priority == highestprio) {
          sameprio[sameprioindex] = sameage[m];
          sameprioindex += 1;
        }
        else {//Sicherheit fuer smallestID-Ueberpruefung
          sameprio[sameprioindex] = MAX_NUMBER_OF_PROCESSES;
        }
      }
      if(sameprioindex == 1) {
        schedulingInfo.age[sameprio[0]] = processes[sameprio[0]].priority;//Alter des ausgewaehlten Prozesses auf Prioritaet zuruecksetzen
        return sameprio[0];
      }
      if(sameprioindex > 1) {//mehr als ein Prozess haben gleiches Alter und Prioritaet
        uint8_t smallestID = MAX_NUMBER_OF_PROCESSES;
        for(uint8_t n=0; n<sameprioindex; n++) {//sucht kleinste Prozess-ID
          if(sameprio[n] < smallestID) {
            smallestID = sameprio[n];
          }
        }
        schedulingInfo.age[smallestID] = processes[smallestID].priority;//Alter des ausgewaehlten Prozesses auf Prioritaet zuruecksetzen
        return smallestID;
      }
    }
    return 0;
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(Process const processes[], ProcessID current) {
    // This is a presence task
    if(processes[current].state == OS_PS_READY) {
      return current;
    }
    else {
      return os_Scheduler_Even(processes, current);
    }
}

/*-------------------------------------------------------------------
  -------------------MLFQ and process queue functions----------------
  -------------------------------------------------------------------*/

/* head produces, tail chases head to consume.
 * tail is "first" and head is "last" */


// Convention: 255 means empty queue slot

#define increment(x) (x = (x + 1) % MAX_NUMBER_OF_PROCESSES)
#define decrement(x) (x = (x - 1) % MAX_NUMBER_OF_PROCESSES)

void pqueue_init (ProcessQueue *queue){
	queue->size = MAX_NUMBER_OF_PROCESSES;
	memset(queue->data, 255, queue->size);
	queue->head = 0;
	queue->tail = 0;
}

void pqueue_reset (ProcessQueue *queue){
	memset(queue->data, 255, queue->size);
	queue->head = 0;
	queue->tail = 0;
}

uint8_t	pqueue_hasNext (ProcessQueue *queue){ // uint8_t as return type is unintuitive as fuck but nvm, do what the script says
	// It's pretty much checking whether the queue is empty (all values 255) or not
	for(uint8_t i = 0; i < queue->size; i++){
		if(queue->data[i] != 255) return 1;
	}
	return 0;
}

ProcessID pqueue_getFirst (ProcessQueue *queue){
	// Tail is supposed to be "first", or the first to be consumed
	return queue->data[queue->tail];
}

void pqueue_dropFirst (ProcessQueue *queue){
	if(pqueue_hasNext(queue)){ // Can't drop if there's nothing to drop
		queue->data[queue->tail] = 255;
		increment(queue->tail);
		if(pqueue_hasNext(queue) == 1){ // There's something that's not 255
			// This while loop makes sure that tail always points to 
			// the next "consumable" value, given that the queue is not empty
			// Reason for doing this is removing PID in the queue might have 
			// left some "holes"
			while(queue->data[queue->tail] == 255){ 
				increment(queue->tail);
			}
		} else {
			queue->tail = queue->head; // Reset to the empty queue
		}
	}
}

void pqueue_append (ProcessQueue *queue, ProcessID pid){
	// Basically just write onto the head then increment head
	// Our implementation makes sure that head always point to
	// an empty slot in the queue, given that it's not full
	if(queue->data[queue->head] == 255){
		queue->data[queue->head] = pid;
		increment(queue->head);
	} else {
		// This case only happens when the queue is full
		// head now points to the value that tail holds
		os_errorPStr("Append fail: head not free");
	}
}

ProcessQueue* MLFQ_getQueue(uint8_t queueID){
	if(queueID < NUMBER_OF_QUEUES){
		return &(pqueue[queueID]);
	} else {
		os_errorPStr("MLFQ: accessed queue > 3");
		return &(pqueue[0]); // Get rid of the warning
	}
}

// Utility functions
uint8_t MLFQ_getDefaultTimeslice(uint8_t queueID){
	switch(queueID){
		case 0: return 1; break;
		case 1: return 2; break;
		case 2: return 4; break;
		case 3: return 8; break;
		default: return 0; break;
	}
	return 0;
}

uint8_t MLFQ_MapToQueue (Priority prio){
	return (3 - (prio >> 6)); // 11 -> Queue 0, 10 -> Queue 1, and so on
}

// To clean dead PID
void pqueue_removePID(ProcessQueue *queue, ProcessID pid){
	for(uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++){
		if(queue->data[i] == pid){
			queue->data[i] = 255;
			if(pqueue_hasNext(queue) == 1){
				while(queue->data[queue->tail] == 255){
					increment(queue->tail);
				}
			} else {
				queue->tail = queue->head;
			}
		}
	}
}

void MLFQ_removePID(ProcessID pid){
	for (uint8_t i = 0; i < NUMBER_OF_QUEUES; i++){
		pqueue_removePID(&pqueue[i], pid);
	}
}

// Handling blocking processes
void MLFQ_moveFirstToLast(ProcessQueue* queue){
	ProcessID first = queue->data[queue->tail];
	pqueue_dropFirst(queue);
	pqueue_append(queue, first);
}

// Find the queue that holds the process
// returns 255 when no queue is found
uint8_t MLFQ_findQueue(ProcessID pid){
	for(uint8_t queueID = 0; queueID < NUMBER_OF_QUEUES; queueID++){
		for(uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++){
			if(pqueue[queueID].data[i] == pid){
				return queueID;
			}
		}
	}
	return 255;
}

// MLFQ Strategy
ProcessID os_Scheduler_MLFQ(Process const processes[], ProcessID current){
	/* If no remaining time of current, demote to the next lower queue */
	if(MLFQ_time[current] == 0){
		uint8_t current_queue = MLFQ_findQueue(current);
		pqueue_removePID(&pqueue[current_queue], current);
		if(current_queue < 3){
			pqueue_append(&pqueue[current_queue + 1], current);
			MLFQ_time[current] = MLFQ_getDefaultTimeslice(current_queue + 1);
		} else if(current_queue == 3) {
			pqueue_append(&pqueue[current_queue], current);
			MLFQ_time[current] = MLFQ_getDefaultTimeslice(current_queue);
		}
	}
	
	/* Clean up dead PIDs */
	for(uint8_t pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++){
		if(os_getProcessSlot(pid)->state == OS_PS_UNUSED){
			MLFQ_removePID(pid);
		}
	}
	
	/* Handle blocking processes. Under the assumption that the code
	 * works as intended, only the "first" (the tail) of a queue can
	 * be in the blocking state. */
	for(uint8_t queueID = 0; queueID < NUMBER_OF_QUEUES; queueID++){
		ProcessID first = pqueue_getFirst(&pqueue[queueID]);
		if(os_getProcessSlot(first)->state == OS_PS_BLOCKED){
			MLFQ_moveFirstToLast(&pqueue[queueID]);
		}
	}
	
	/*---------------- Choosing the next process ----------------*/
	for(uint8_t qid = 0; qid < NUMBER_OF_QUEUES; qid++){
		ProcessID first = pqueue_getFirst(&pqueue[qid]);
		
		// First condition to make sure the queue is not empty, thus there's a valid process
		// Second condition to make sure the process is not blocked (if that happens then the queue consists solely of one blocking process)
		// Because we iterate from 0 to 3, the specified requirements are met
		if(pqueue_hasNext(&pqueue[qid]) && os_getProcessSlot(first)->state == OS_PS_READY){
			MLFQ_time[first]--; // decrement time slice of the chosen process
			return first;
		}
	}
	
	// If the code ever reaches here, it means that there's no choosable process in all queues
	return 0;
}
