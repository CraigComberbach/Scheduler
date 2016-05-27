//Add individual task timing statistics
//Add global timing statistics
//Doesn't fully support a limited number of occurances

/**************************************************************************************************
Authours:				Craig Comberbach
Target Hardware:		PIC24F
Chip resources used:	Uses a timer
Code assumptions:
Purpose:				

Version History:

**************************************************************************************************/
/*************    Header Files    ***************/
#include "Config.h"
#include "Scheduler.h"

/************* Semantic Versioning***************/
#if SCHEDULER_MAJOR != 0
	#error "Scheduler library has had a change that loses some previously supported functionality"
#elif SCHEDULER_MINOR != 0
	#error "Scheduler library has new features that this code may benefit from"
#elif SCHEDULER_PATCH != 0
	#error "Scheduler library has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
#endif

/************Arbitrary Functionality*************/
/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********State Machine Definitions************/
/*************  Global Variables  ***************/
#ifdef CONSERVE_MEMORY
	uint16_t schedulerPeriod_mS;
#else
	uint32_t schedulerPeriod_mS;
#endif

struct SCHEDULED_TASKS
{
	#ifdef CONSERVE_MEMORY
		void (*task)(uint16_t);
		uint16_t initialDelay_mS;
		uint16_t period_mS;
		uint16_t elapsedTime_mS;
		uint16_t recurrence;
		uint16_t recurrenceCount;
		uint16_t minExecutionTime_uS;
		uint16_t avgExecutionTime_uS;
		uint16_t maxExecutionTime_uS;
		uint16_t currentExecutionTime_uS;
	#else
		void (*task)(uint32_t);
		uint32_t initialDelay_mS;
		uint32_t period_mS;
		uint32_t elapsedTime_mS;
		uint32_t recurrence;
		uint32_t recurrenceCount;
		uint32_t minExecutionTime_uS;
		uint32_t avgExecutionTime_uS;
		uint32_t maxExecutionTime_uS;
		uint32_t currentExecutionTime_uS;
	#endif
} scheduledTasks[NUMBER_OF_SCHEDULED_TASKS];
uint32_t minGlobalExecutionTime_uS;
uint32_t avgGlobalExecutionTime_uS;
uint32_t maxGlobalExecutionTime_uS;
int16_t delayFlag = 0;

/*************Function  Prototypes***************/
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void);

/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

void Task_Master_2000(void)
{
	int16_t loop;
	
	for(loop = 0; loop < NUMBER_OF_SCHEDULED_TASKS; ++loop)
	{
		ClrWdt();
		if(scheduledTasks[loop].initialDelay_mS <= schedulerPeriod_mS)
		{
			if(scheduledTasks[loop].recurrenceCount > 0)
			{
				if(scheduledTasks[loop].elapsedTime_mS >= scheduledTasks[loop].period_mS)
				{
//Record time for future calculation
					scheduledTasks[loop].task(scheduledTasks[loop].elapsedTime_mS);			//Run the current task, send the time since last execution
//Snap current time for min/avg/max time evaluation
					if(++scheduledTasks[loop].recurrenceCount == 0)
						scheduledTasks[loop].recurrenceCount = 1;
				}
				else
				{
					scheduledTasks[loop].elapsedTime_mS += schedulerPeriod_mS;				//Continue towards NEXT execution
				}
			}
			else
			{
//Record time for future calculation
				scheduledTasks[loop].task(0);												//Run the current task, indicate this is the first time running (0mS)
//Snap current time for min/avg/max time evaluation
				scheduledTasks[loop].recurrenceCount = 1;
				
			}
		}
		else
		{
			scheduledTasks[loop].initialDelay_mS -= schedulerPeriod_mS;						//Continue towards FIRST execution
		}
			
	}

	delayFlag = 0;

	return;
}

void Initialize_Scheduler(uint32_t newPeriod_mS)
{
//	I need Instruction clock (FCY Hz)
//	I need period (mS))
//	I need Number of schedulable tasks
	if(newPeriod_mS != 0)
		schedulerPeriod_mS = newPeriod_mS;
	else
		while(1);//TODO - DEBUG ME! I should never execute

	return;
}

void Schedule_Task(enum SCHEDULER_DEFINITIONS taskName, void (*newTask)(uint32_t), uint16_t newInitialDelay_mS, uint16_t newPeriod_mS, uint16_t newRepetitions)
{
	//Task Information
	if(*newTask  != (void*)0)
		scheduledTasks[taskName].task = newTask;
	else
		while(1);//TODO - DEBUG ME! I should never execute

	//Timing Information
	scheduledTasks[taskName].initialDelay_mS = newInitialDelay_mS;
	scheduledTasks[taskName].period_mS = newPeriod_mS;
	scheduledTasks[taskName].elapsedTime_mS = 0;

	//Recurrence Information
	scheduledTasks[taskName].recurrence = newRepetitions;
	scheduledTasks[taskName].recurrenceCount = 0;

	//Runtime Statistics Information
	scheduledTasks[taskName].currentExecutionTime_uS = 0;
	scheduledTasks[taskName].minExecutionTime_uS = ~0;
	scheduledTasks[taskName].avgExecutionTime_uS = 0;
	scheduledTasks[taskName].maxExecutionTime_uS = 0;
	
	return;
}

int8_t Waiting_To_Run_Tasks(void)
{
	return delayFlag;
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
	delayFlag = 1;
	TMR1 = 0;
	IFS0bits.T1IF = 0;
	return;
}
