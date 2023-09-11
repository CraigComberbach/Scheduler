/**************************************************************************************************
Version History:
v0.1.0	2016-05-30	Craig Comberbach	Compiler: XC16 v1.11
 * Added limited recurrence
 * Refactored the Task Master function to be simpler
 * Added Auto-Magic calculation of Timer1 during initialization
v0.0.0	2016-05-26	Craig Comberbach	Compiler: XC16 v1.11
 * First version - Most functionality implemented
 * Does not implement limited recurrence tasks, only permanent ones
 
**************************************************************************************************/
/*************    Header Files    ***************/
#include <stdint.h>
#include "Config.h"
#include "Scheduler.h"

/*************Semantic  Versioning***************/
#if SCHEDULER_MAJOR != 0
	#error "Scheduler library has had a change that loses some previously supported functionality"
#elif SCHEDULER_MINOR != 2
	#error "Scheduler library has new features that this code may benefit from"
#elif SCHEDULER_PATCH != 0
	#error "Scheduler library has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
#endif

/*************   Magic  Numbers   ***************/
#define NULL_POINTER	(void*)0

#define TASK_PROFILING_ENABLED

/*************    Enumeration     ***************/
/***********  Structure Definitions  ************/
/***********State Machine Definitions************/
/*************  Global Variables  ***************/
uint32_t schedulerPeriod_uS;

struct SCHEDULED_TASKS
{
	void (*task)(uint32_t);
	uint32_t period_uS;
	uint32_t countDown_uS;
	uint32_t recurrenceTarget;
	uint32_t recurrenceCount;
	#ifdef TASK_PROFILING_ENABLED
		uint32_t minExecutionTime_FCYticks;
		uint32_t sumExecutionTime_FCYticks;
		uint32_t avgExecutionTime_FCYticks;
		uint32_t maxExecutionTime_FCYticks;
		uint32_t currentExecutionTime_FCYticks;
	#endif
} scheduledTasks[NUMBER_OF_SCHEDULED_TASKS];

uint32_t minGlobalExecutionTime_uS;
uint32_t avgGlobalExecutionTime_uS;
uint32_t maxGlobalExecutionTime_uS;
int16_t delayFlag = 0;

/*************Function  Prototypes***************/
/************* Main Body Of Code  ***************/
void Scheduler_Run(void)
{
	int16_t taskIndex;
	int16_t time;
	
	for(taskIndex = 0; taskIndex < NUMBER_OF_SCHEDULED_TASKS; ++taskIndex)
	{
		if(scheduledTasks[taskIndex].task == NULL_POINTER)
            continue;
        
		if(scheduledTasks[taskIndex].countDown_uS <= schedulerPeriod_uS)
		{
			if((scheduledTasks[taskIndex].recurrenceCount < scheduledTasks[taskIndex].recurrenceTarget) || (scheduledTasks[taskIndex].recurrenceTarget == PERMANENT_TASK))
			{
				//Document how many times this task has run (safely rolls over)
				if(++scheduledTasks[taskIndex].recurrenceCount == 0)
					scheduledTasks[taskIndex].recurrenceCount = 1;

				scheduledTasks[taskIndex].countDown_uS = scheduledTasks[taskIndex].period_uS;	//Reset for next time
				time = TMR1;//Record when the task started
				scheduledTasks[taskIndex].task(scheduledTasks[taskIndex].period_uS);			//Run the current task, send the time since last execution
				time = TMR1 - time;//Record how long the task took

				//Task profiling
				#ifdef TASK_PROFILING_ENABLED
					//Minimum execution Time
					if(time < scheduledTasks[taskIndex].minExecutionTime_FCYticks)
						scheduledTasks[taskIndex].minExecutionTime_FCYticks = time;

					//Maximum Execution time
					if(time > scheduledTasks[taskIndex].maxExecutionTime_FCYticks)
						scheduledTasks[taskIndex].maxExecutionTime_FCYticks = time;

					//Average Execution Time
					scheduledTasks[taskIndex].sumExecutionTime_FCYticks += time;
					scheduledTasks[taskIndex].avgExecutionTime_FCYticks = scheduledTasks[taskIndex].sumExecutionTime_FCYticks / scheduledTasks[taskIndex].recurrenceCount;
				#endif
			}
		}
		else
			scheduledTasks[taskIndex].countDown_uS -= schedulerPeriod_uS;
	}

	delayFlag = 0;

	return;
}

void Scheduler_Initialize(uint32_t newPeriod_uS)
{
	uint32_t period;
    uint8_t loopIndex = 0;

	//Ensure the period makes sense
	if(newPeriod_uS != 0)
		schedulerPeriod_uS = newPeriod_uS;
	else
		while(1)//TODO - DEBUG ME! I should never execute
			asm("ClrWdt");

	period = FCY_MHz;
	period *= newPeriod_uS;
	
	if(period > 65535)
		while(1);//This period can never run, we only have 16 bits of register

    for(loopIndex = 0; loopIndex < NUMBER_OF_SCHEDULED_TASKS; loopIndex++)
    {
        scheduledTasks[loopIndex].task = NULL_POINTER;
    }
	
	return;
}

void Scheduler_Add_Task(enum SCHEDULER_DEFINITIONS taskName, void (*newTask)(uint32_t), uint32_t newInitialDelay_uS, uint32_t newPeriod_uS, uint16_t newRepetitions)
{
	//Task Information
	if(*newTask  != NULL_POINTER)
		scheduledTasks[taskName].task = newTask;
	else
		while(1)//TODO - DEBUG ME! I should never execute
			asm("ClrWdt");

	if(newPeriod_uS < schedulerPeriod_uS)
		while(1)//TODO - DEBUG ME! I should never execute
			asm("ClrWdt");

	if(newInitialDelay_uS < schedulerPeriod_uS)
		while(1)//TODO - DEBUG ME! I should never execute
			asm("ClrWdt");

	//Timing Information
	scheduledTasks[taskName].countDown_uS = newInitialDelay_uS;
	scheduledTasks[taskName].period_uS = newPeriod_uS;

	//Recurrence Information
	scheduledTasks[taskName].recurrenceTarget = newRepetitions;
	scheduledTasks[taskName].recurrenceCount = 0;

	#ifdef TASK_PROFILING_ENABLED
		//Runtime Statistics Information
		scheduledTasks[taskName].currentExecutionTime_FCYticks = 0;
		scheduledTasks[taskName].minExecutionTime_FCYticks = ~0;
		scheduledTasks[taskName].avgExecutionTime_FCYticks = 0;
		scheduledTasks[taskName].maxExecutionTime_FCYticks = 0;
	#endif

	return;
}

int8_t Waiting_To_Run_Tasks(void)
{
	return delayFlag;
}

void Scheduler_Expedite_Task(enum SCHEDULER_DEFINITIONS taskToExpedite)
{
    if(taskToExpedite < NUMBER_OF_SCHEDULED_TASKS)
        scheduledTasks[taskToExpedite].countDown_uS = 0;
    
    return;
}

void Scheduler_Tick_Interupt(void)
{
	delayFlag = 1;
	return;
}
