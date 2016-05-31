//Add individual task timing statistics
//Add global timing statistics

/**************************************************************************************************
Authours:				Craig Comberbach
Target Hardware:		PIC24F
Chip resources used:	Uses timer1
Code assumptions:		
Purpose:				Allows easy scheduling of tasks with minimal overhead

Version History:
v0.1.0	2016-05-30	Craig Comberbach	Compiler: XC16 v1.11	IDE: MPLABx 3.30	Tool: ICD3		Computer: Intel Core2 Quad CPU 2.40 GHz, 5 GB RAM, Windows 10 Home 64-bit
 Added limited recurrence
 Refactored the Task Master function to be simpler
 Added Auto-Magic? calculation of Timer1 during initialization
v0.0.0	2016-05-26	Craig Comberbach	Compiler: XC16 v1.11	IDE: MPLABx 3.30	Tool: ICD3		Computer: Intel Core2 Quad CPU 2.40 GHz, 5 GB RAM, Windows 10 Home 64-bit
 First version - Most functionality implemented
 Does not implement limited recurrence tasks, only permanent ones
 
**************************************************************************************************/
/*************    Header Files    ***************/
#include "Config.h"
#include "Scheduler.h"

/************* Semantic Versioning***************/
#if SCHEDULER_MAJOR != 0
	#error "Scheduler library has had a change that loses some previously supported functionality"
#elif SCHEDULER_MINOR != 1
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
		uint16_t period_mS;
		uint16_t countDown_mS;
		uint16_t recurrence;
		uint16_t recurrenceCount;
		uint16_t minExecutionTime_uS;
		uint16_t avgExecutionTime_uS;
		uint16_t maxExecutionTime_uS;
		uint16_t currentExecutionTime_uS;
	#else
		void (*task)(uint32_t);
		uint32_t period_mS;
		uint32_t countDown_mS;
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
		if(scheduledTasks[loop].countDown_mS <= schedulerPeriod_mS)
		{
			if((scheduledTasks[loop].recurrenceCount < scheduledTasks[loop].recurrence) || (scheduledTasks[loop].recurrence == 0))
			{
				if(++scheduledTasks[loop].recurrenceCount == 0)
					scheduledTasks[loop].recurrenceCount = 1;

				scheduledTasks[loop].countDown_mS = scheduledTasks[loop].period_mS;	//Reset for next time
				scheduledTasks[loop].task(scheduledTasks[loop].period_mS);			//Run the current task, send the time since last execution
			}
		}
		else
			scheduledTasks[loop].countDown_mS -= schedulerPeriod_mS;
	}

	delayFlag = 0;

	return;
}

void Initialize_Scheduler(uint32_t newPeriod_mS)
{
	//Ensure the period makes sense
	if(newPeriod_mS != 0)
		schedulerPeriod_mS = newPeriod_mS;
	else
		while(1)//TODO - DEBUG ME! I should never execute
			ClrWdt();

	//Auto-Magically setup Timer1
	PR1				= (newPeriod_mS * FCY_kHz) / 8;//500 = 1 mS assuming Prescaler /8
	IEC0bits.T1IE	= 1;
	T1CONbits.TCS	= 0;
//	T1CONbits.TSYNC	= Ignored
	T1CONbits.TCKPS	= 1; //1 = Fcy/8
	T1CONbits.TGATE	= 0;
	T1CONbits.TSIDL	= 0;
	T1CONbits.TON	= 1;
	
	return;
}

void Schedule_Task(enum SCHEDULER_DEFINITIONS taskName, void (*newTask)(uint32_t), uint16_t newInitialDelay_mS, uint16_t newPeriod_mS, uint16_t newRepetitions)
{
	//Task Information
	if(*newTask  != (void*)0)
		scheduledTasks[taskName].task = newTask;
	else
		while(1)//TODO - DEBUG ME! I should never execute
			ClrWdt();

	if(newPeriod_mS < schedulerPeriod_mS)
		while(1)//TODO - DEBUG ME! I should never execute
			ClrWdt();

	if(newInitialDelay_mS < schedulerPeriod_mS)
		while(1)//TODO - DEBUG ME! I should never execute
			ClrWdt();

	//Timing Information
	scheduledTasks[taskName].countDown_mS = newInitialDelay_mS;
	scheduledTasks[taskName].period_mS = newPeriod_mS;

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
