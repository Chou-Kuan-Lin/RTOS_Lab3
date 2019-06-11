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
#define  N_TASKS                         3       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;

OS_EVENT     *R1;
OS_EVENT     *R2;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task1(void *data);                       /* Function prototypes of tasks                  */
        void  Task2(void *data);                       /* Function prototypes of tasks                  */
        void  Task3(void *data);                       /* Function prototypes of tasks                  */
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
    INT8U         err;
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */
    
    R1 = OSMutexCreate(1, &err);
    R2 = OSMutexCreate(2, &err);



    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
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
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
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
    OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], 3);
    OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], 4);
    OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE - 1], 5);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  Task1 ()
{
    INT8U         err;
    OSTimeDly(8);
    
    OSTCBCur -> compTime = 2;
    while ((OSTCBCur -> compTime) > 0) { }

    OSTCBCur -> compTime = 2;
    OSMutexPend(R1, 0, &err);
    while (OSTCBCur -> compTime > 0) { }
    
    OSTCBCur -> compTime = 2;
    OSMutexPend(R2, 0, &err);
    while (OSTCBCur -> compTime > 0) { }
    OSMutexPost(R1);
    OSMutexPost(R2);

    OSTaskDel(OS_PRIO_SELF);
}

void  Task2 (void *pdata)
{
    INT8U         err;
    OSTimeDly(4);
    

    OSTCBCur -> compTime = 2;    
    while ((OSTCBCur -> compTime) > 0) {}

    OSTCBCur -> compTime = 5;
    
    OSMutexPend(R2, 0, &err);
    while (OSTCBCur -> compTime > 0) { }
    OSMutexPost(R2);
    // printf("Finish %u \n", OSTCBCur->OSTCBPrio);
    
    
    OSTaskDel(OS_PRIO_SELF);
}

void  Task3 (void *pdata)
{
    INT8U         err;
    OSTimeDly(0);
    
    OSTCBCur -> compTime = 2;
    while (OSTCBCur -> compTime > 0) { }

    OSTCBCur -> compTime = 7;
   
    OSMutexPend(R1, 0, &err);
    while (OSTCBCur -> compTime > 0) { }
    OSMutexPost(R1);
    
    // printf("Finish %u \n", OSTCBCur->OSTCBPrio);
    
    OSTaskDel(OS_PRIO_SELF);
}