/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                         2       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task1(void *data);                       /* Function prototypes of tasks                  */
        void  Task2(void *data);                       /* Function prototypes of tasks                  */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */

    
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);

    OSTimeSet(0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    TaskStartCreateTasks();                                /* Create all the application tasks         */

    for (;;) {
        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 2, 0);             
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
    OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], 1);
    OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], 2);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  Task1 ()
{
    INT32U start = OSTimeGet();
    INT32U end;
    INT32U toDelay;
    
    OS_ENTER_CRITICAL();
    OSTCBCur -> compTime = 1;
    OSTCBCur -> period = start + 3;
    OS_EXIT_CRITICAL();

    while (1) {       
        while(1) {
            OS_ENTER_CRITICAL();
            if ((OSTCBCur -> compTime) <= 0) {
                OS_EXIT_CRITICAL();
                break;
            }
            OS_EXIT_CRITICAL();
        }

        end = OSTimeGet();
       
        OS_ENTER_CRITICAL();
        toDelay = (OSTCBCur -> period) - end;
        start += 3;
        OSTCBCur -> period += 3;
        (OSTCBCur -> compTime) = 1;
        // printf("Task1 nextStart = %lu, End %lu, nextEnd = %lu\n", start, end, OSTCBCur->period);
        OS_EXIT_CRITICAL();
        
        if (toDelay > 0) OSTimeDly(toDelay);
    }
}

void  Task2 ()
{
    INT32U start = OSTimeGet();
    INT32U end;
    INT32U toDelay;
   
    OS_ENTER_CRITICAL();
    OSTCBCur -> compTime = 3;
    OSTCBCur -> period = start + 5;
    OS_EXIT_CRITICAL();

    while (1) {
        while(1)
        {
            OS_ENTER_CRITICAL();
            if ((OSTCBCur -> compTime) <= 0) {
                OS_EXIT_CRITICAL();
                break;
            }
            OS_EXIT_CRITICAL();
        }
        end = OSTimeGet();

        OS_ENTER_CRITICAL();
        toDelay = (OSTCBCur -> period) - end;
        start += 5;
        OSTCBCur -> period += 5;
        (OSTCBCur -> compTime) = 3;
        printf("Task2 nextStart = %lu, End %lu, nextEnd = %lu\n", start, end, OSTCBCur->period);
        OS_EXIT_CRITICAL();
        
        if (toDelay > 0) OSTimeDly(toDelay);
    }
}