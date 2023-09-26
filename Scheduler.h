#ifndef SCHEDULER_H
#define	SCHEDULER_H

/*
 * Instructions for adding to a new project:
 * 
 * How to Use:
 * The scheduler is able to use functions of the following format
 *   void Name_Of_Routine(unsigned long time_uS)
 */
/**********Add to config.c************/
/*
enum SCHEDULER_DEFINITIONS
{
	TASK_,
	NUMBER_OF_SCHEDULED_TASKS
};

	//Scheduler Library
	#define SCHEDULER_MAJOR	0
	#define SCHEDULER_MINOR	1
	#define SCHEDULER_PATCH	0
 */

/**********Add to config.h************/
/*
#ifndef SCHEDULER_LIBRARY
	#error "Scheduler library was not found"
#endif
 */

/********Semantic Versioning**********/
#define SCHEDULER_LIBRARY

/***********Magic Numbers*************/
#define PERMANENT_TASK	0
#define us				1
#define ms				1000
#define s				1000000

/************Enumeration**************/
/****Module Function Prototypes*******/
void Scheduler_Initialize(void);
void Scheduler_Add_Profiling_Clock(volatile uint16_t *profilingTimer);
void Scheduler_Set_Period_us(uint32_t newPeriod_uS);

void Scheduler_Run_Tasks(void);

void Scheduler_Add_Task(enum SCHEDULER_DEFINITIONS task, void (*newTask)(uint32_t), uint32_t initialDelay_uS, uint32_t taskPeriod_uS, uint16_t numberOfRepetitions);
void Scheduler_Expedite_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_Start_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_Pause_Task(enum SCHEDULER_DEFINITIONS task);
void Scheduler_End_Task(enum SCHEDULER_DEFINITIONS task);

void Scheduler_Timer_Interupt(void);

#endif	/* SCHEDULER_H */

