#ifndef SCHEDULER_H
#define	SCHEDULER_H

/*
Instructions for adding to a new project:
*/

/***********Add to config file header************/
/*
//A2D Library
#define SCHEDULER_MAJOR	0
#define SCHEDULER_MINOR	0
#define SCHEDULER_PATCH	0
*/

/***************Add to config file***************/
/*
#ifndef SCHEDULER_LIBRARY
	#error "You need to include the Scheduler library for this code to compile"
#endif
 */

/************* Semantic Versioning***************/
#define SCHEDULER_LIBRARY

/*************   Magic  Numbers   ***************/
#define PERMANENT_TASK	0

/*************    Enumeration     ***************/
/***********State Machine Definitions************/
/*************Function  Prototypes***************/
void Task_Master_2000(void);
void Initialize_Scheduler(uint32_t newPeriod_mS);
void Schedule_Task(enum SCHEDULER_DEFINITIONS taskDuJour, void (*newTask)(uint32_t), uint16_t newInitialDelay_mS, uint16_t newPeriod_mS, uint16_t newRepetitions);
int8_t Waiting_To_Run_Tasks(void);

#endif	/* SCHEDULER_H */

