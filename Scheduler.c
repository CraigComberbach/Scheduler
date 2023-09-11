/**************************************************************************************************
Version History:
v1.0.0	2023-09-11	Craig Comberbach	Compiler: XC16 v2.00
 * The schedule now handles the infinite while loop
 * Task profiling is now done with an inline algorithim instead of sum and divide
 * The Scheduler timer is now handled by the timer library
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
#include "Timers.h"

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

/*************    Enumeration     ***************/
enum SCHEDULER_STATE
{
	RUNNING,
	PAUSED,
	FINISHED
};

/***********  Structure Definitions  ************/
/***********State Machine Definitions************/
/*************  Global Variables  ***************/
uint32_t schedulerPeriod_uS = 0;

struct SCHEDULED_TASKS
{
	void (*task)(uint32_t);
	enum SCHEDULER_STATE state;
	uint32_t period_uS;
	uint32_t countDown_uS;
	uint32_t recurrenceTarget;
	uint32_t recurrenceCount;
	
	//Task Profiling
	uint16_t minExecutionTime_FCYticks;
	uint16_t avgExecutionTime_FCYticks;
	uint16_t maxExecutionTime_FCYticks;
} scheduledTasks[NUMBER_OF_SCHEDULED_TASKS];

volatile uint16_t *TimerForProfiling;

int16_t delayFlag = 0;

/*************Function  Prototypes***************/
void Run_Tasks(void);
void Task_Profiling(int32_t time, uint16_t task);

/************* Main Body Of Code  ***************/
void Scheduler_Run_Tasks(void)
{
	while(1)
	{
		while(delayFlag)
		{
			asm("ClrWdt");
			Run_Tasks();
		}
	}
	
	return;
}

void Run_Tasks(void)
{
	uint16_t taskIndex;
	int32_t timeTaken;
	

	for(taskIndex = 0; taskIndex < NUMBER_OF_SCHEDULED_TASKS; ++taskIndex)
	{
		if(scheduledTasks[taskIndex].task == NULL_POINTER)
            continue;
        
		if(scheduledTasks[taskIndex].state != RUNNING)
			continue;
		
		if(scheduledTasks[taskIndex].countDown_uS <= schedulerPeriod_uS)
		{
			if((scheduledTasks[taskIndex].recurrenceCount < scheduledTasks[taskIndex].recurrenceTarget) || (scheduledTasks[taskIndex].recurrenceTarget == PERMANENT_TASK))
			{
				//Document how many times this task has run (safely rolls over)
				if(++scheduledTasks[taskIndex].recurrenceCount == 0)
					scheduledTasks[taskIndex].recurrenceCount = 1;

				scheduledTasks[taskIndex].countDown_uS = scheduledTasks[taskIndex].period_uS;	//Reset for next time
				timeTaken = *TimerForProfiling;//Record when the task started
				scheduledTasks[taskIndex].task(scheduledTasks[taskIndex].period_uS);			//Run the current task, send the time since last execution
				timeTaken = *TimerForProfiling - timeTaken;//Record how long the task took

				Task_Profiling(timeTaken, taskIndex);
			}
		}
		else
			scheduledTasks[taskIndex].countDown_uS -= schedulerPeriod_uS;
	}

	delayFlag = 0;

	return;
}

void Task_Profiling(int32_t time, uint16_t task)
{
	//Minimum execution Time
	if(time < scheduledTasks[task].minExecutionTime_FCYticks)
		scheduledTasks[task].minExecutionTime_FCYticks = time;

	//Maximum Execution time
	if(time > scheduledTasks[task].maxExecutionTime_FCYticks)
		scheduledTasks[task].maxExecutionTime_FCYticks = time;

	//Average Execution Time
	scheduledTasks[task].avgExecutionTime_FCYticks = scheduledTasks[task].avgExecutionTime_FCYticks + (time - scheduledTasks[task].avgExecutionTime_FCYticks) / (int32_t)scheduledTasks[task].recurrenceCount;
	return;
}

void Scheduler_Initialize(uint32_t newPeriod_uS)
{
    uint8_t task;

	schedulerPeriod_uS = newPeriod_uS;
	
    for(task = 0; task < NUMBER_OF_SCHEDULED_TASKS; task++)
    {
        scheduledTasks[task].task = NULL_POINTER;
		scheduledTasks[task].state = FINISHED;
    }
	
	return;
}

void Scheduler_Add_Profiling_Clock(volatile uint16_t *profilingTimer)
{
	TimerForProfiling = profilingTimer;
	
	return;
}

void Scheduler_Add_Task(enum SCHEDULER_DEFINITIONS task, void (*newTask)(uint32_t), uint32_t initialDelay_uS, uint32_t taskPeriod_uS, uint16_t numberOfRepetitions)
{
	if(*newTask  == NULL_POINTER)
		while(1);//TODO - error handling

	if(taskPeriod_uS < schedulerPeriod_uS)
		while(1);//TODO - error handling

	if(initialDelay_uS < schedulerPeriod_uS)
		while(1);//TODO - error handling

	scheduledTasks[task].task = newTask;
	scheduledTasks[task].state = RUNNING;

	scheduledTasks[task].countDown_uS = initialDelay_uS;
	scheduledTasks[task].period_uS = taskPeriod_uS;

	scheduledTasks[task].recurrenceTarget = numberOfRepetitions;
	scheduledTasks[task].recurrenceCount = 0;

	scheduledTasks[task].minExecutionTime_FCYticks = UINT16_MAX;
	scheduledTasks[task].avgExecutionTime_FCYticks = 0;
	scheduledTasks[task].maxExecutionTime_FCYticks = 0;

	return;
}

void Scheduler_Expedite_Task(enum SCHEDULER_DEFINITIONS task)
{
    if(task < NUMBER_OF_SCHEDULED_TASKS)
	{
        scheduledTasks[task].countDown_uS = 0;
	}
    
    return;
}

void Scheduler_Tick_Interupt(void)
{
	delayFlag = 1;
	return;
}

void Scheduler_Start_Task(enum SCHEDULER_DEFINITIONS task)
{
	if((task < NUMBER_OF_SCHEDULED_TASKS) && (scheduledTasks[task].state == PAUSED))
	{
		scheduledTasks[task].state = RUNNING;
	}

	return;
}

void Scheduler_Pause_Task(enum SCHEDULER_DEFINITIONS task)
{
	if((task < NUMBER_OF_SCHEDULED_TASKS) && (scheduledTasks[task].state == RUNNING))
	{
		scheduledTasks[task].state = PAUSED;
	}

	return;
}

void Scheduler_End_Task(enum SCHEDULER_DEFINITIONS task)
{
	if(task < NUMBER_OF_SCHEDULED_TASKS)
	{
		scheduledTasks[task].minExecutionTime_FCYticks = 0;
		scheduledTasks[task].avgExecutionTime_FCYticks = 0;
		scheduledTasks[task].maxExecutionTime_FCYticks = 0;
		scheduledTasks[task].countDown_uS = 0;
		scheduledTasks[task].period_uS = 0;
		scheduledTasks[task].recurrenceCount = 0;
		scheduledTasks[task].recurrenceTarget = 0;
		scheduledTasks[task].state = FINISHED;
		scheduledTasks[task].task = NULL_POINTER;
	}
	
	return;
}
