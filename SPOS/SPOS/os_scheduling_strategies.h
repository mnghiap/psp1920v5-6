/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduling strategies.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H

#include "os_scheduler.h"
#include "defines.h"

//! Structure used to store specific scheduling informations such as a time slice
// This is a presence task
typedef struct {
  uint8_t timeSlice;//Zeitscheibe fuer RoundRobin
  Age age[MAX_NUMBER_OF_PROCESSES];//Prozessalter fuer InactiveAging
} SchedulingInformation;

//! Process Queue for MLFQ
typedef struct {
	ProcessID data[MAX_NUMBER_OF_PROCESSES];
	uint8_t size;
	uint8_t head;
	uint8_t tail;
} ProcessQueue;

//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(ProcessID id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(SchedulingStrategy strategy);

//! Even strategy
ProcessID os_Scheduler_Even(Process const processes[], ProcessID current);

//! Random strategy
ProcessID os_Scheduler_Random(Process const processes[], ProcessID current);

//! RoundRobin strategy
ProcessID os_Scheduler_RoundRobin(Process const processes[], ProcessID current);

//! InactiveAging strategy
ProcessID os_Scheduler_InactiveAging(Process const processes[], ProcessID current);

//! RunToCompletion strategy
ProcessID os_Scheduler_RunToCompletion(Process const processes[], ProcessID current);

//! MLFQ
ProcessID os_Scheduler_MLFQ(Process const processes[], ProcessID current);

//! Process queue functions and constants

#define NUMBER_OF_QUEUES 4

void os_initSchedulingInformation(void);

void pqueue_init (ProcessQueue *queue);

void pqueue_reset (ProcessQueue *queue);

uint8_t	pqueue_hasNext (ProcessQueue *queue);

ProcessID pqueue_getFirst (ProcessQueue *queue);

void pqueue_dropFirst (ProcessQueue *queue);

void pqueue_append (ProcessQueue *queue, ProcessID pid);

ProcessQueue* MLFQ_getQueue(uint8_t queueID);

#endif
