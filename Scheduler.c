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
#if SCHEDULER_MAJOR != 1
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
/*************Function  Prototypes***************/
/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/
