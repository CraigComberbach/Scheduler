#ifndef SCHEDULER_H
#define	SCHEDULER_H

/*
 * Instructions for adding to a new project:
 * 
 * How to Use:
 * The scheduler is able to use functions of the following format
 *   void Name_Of_Routine(unsigned long time_uS)
*/
/***********Add to config file header************/
/*
//Scheduler Library
#define SCHEDULER_MAJOR	0
#define SCHEDULER_MINOR	2
#define SCHEDULER_PATCH	0

enum SCHEDULER_DEFINITIONS
{
	TASK_,
	NUMBER_OF_SCHEDULED_TASKS
};
*/

/***************Add to config file***************/
/*
#ifndef SCHEDULER_LIBRARY
	#error "You need to include the Scheduler library for this code to compile"
#endif
 */

/*************Semantic  Versioning***************/
#define SCHEDULER_HAL

/*************   Magic  Numbers   ***************/
#define PERMANENT_TASK	0
#define us				1
#define ms				1000
#define s				1000000

/*************    Enumeration     ***************/
/***********State Machine Definitions************/
/*************Function  Prototypes***************/
void Scheduler_Run_Tasks(void);
void Scheduler_Initialize(uint32_t newPeriod_uS);
void Scheduler_Add_Profiling_Clock(volatile uint16_t *profilingTimer);
void Scheduler_Add_Task(enum SCHEDULER_DEFINITIONS task, void (*newTask)(uint32_t), uint32_t initialDelay_uS, uint32_t taskPeriod_uS, uint16_t numberOfRepetitions);
void Scheduler_Expedite_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_Tick_Interupt(void);
void Scheduler_Start_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_Pause_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_End_Task(enum SCHEDULER_DEFINITIONS task);

/* New functions to implement
 *  StartTask
 *  PauseTask
 *  EndTask
 * 
 * Additional functionality to add to tasks
 *  Arbitrary pointer, normally not used, but can be assigned as needed
 */


#endif	/* SCHEDULER_H */

