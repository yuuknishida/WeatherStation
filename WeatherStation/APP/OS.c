/*
 * OS.c
 *
 *  Created on: Feb 27, 2026
 *      Author: ynish
 *
 *  Cooperative RTOS implementation for scheduling periodic tasks
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "tm4c123gh6pm.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "OS.h"

uint32_t taskStk[MAX_TASKS][STACK_SIZE]  __attribute__((aligned(8)));
task_t tasksTbl[MAX_TASKS];
uint8_t taskCount = 0;


task_t *taskHead = NULL;
task_t *currentTask = NULL;

static void OS_TaskExit(void)
{
    if (currentTask != NULL)
        currentTask->state = BLOCKED;
    OS_Yield();
    while (1) {}
}

static void Task_Idle(void)
{
    while(1) { }   // just spin, never sleeps
}

/*******************************************************************
*                               OS_Init()
*
* Initializes RTOS data structures and starts SysTick timer.
*********************************************************************/
void OS_Init()
{
    taskCount = 0;
    OS_AddTask(Task_Idle, 0);
}

/*******************************************************************
*                               OS_AddTask()
*
* Adds a task to the RTOS scheduler.
* Parameters:
*   taskFunc - pointer to function to execute
*   priority - task priority (0 = highest)
*
*********************************************************************/
void OS_AddTask(void (*taskFunc)(void), uint16_t ticks)
{
    if (taskCount >= MAX_TASKS) return;

    task_t *task = &tasksTbl[taskCount];
    uint32_t *stk = &taskStk[taskCount][STACK_SIZE];

    *(--stk) = 0x01000000;              // xPSR
    *(--stk) = (uint32_t)taskFunc;      // PC
    *(--stk) = (uint32_t)OS_TaskExit;   // LR
    *(--stk) = 0x12121212;              // R12
    *(--stk) = 0x03030303;              // R3
    *(--stk) = 0x02020202;              // R2
    *(--stk) = 0x01010101;              // R1
    *(--stk) = 0x00000000;              // R0

    *(--stk) = 0x11111111;              // R11
    *(--stk) = 0x10101010;              // R10
    *(--stk) = 0x09090909;              // R9
    *(--stk) = 0x08080808;              // R8
    *(--stk) = 0x07070707;              // R7
    *(--stk) = 0x06060606;              // R6
    *(--stk) = 0x05050505;              // R5
    *(--stk) = 0x04040404;              // R4

    task->stkPtr = stk;

    task->sleepPeriod = ticks;
    task->state = READY;
    task->sleepTick = ticks;
    task->taskFunc = taskFunc;
    task->id = taskCount;

    taskCount++;
}

/*******************************************************************
*                               SysTick_Handler()
*
* Interrupt handler for SysTick timer (1 ms tick).
* Increments elapsedTime for all registered tasks.
*********************************************************************/
void SysTick_Handler(void)
{
    for (int i = 0;i < taskCount; i++) {
        if (tasksTbl[i].sleepTick > 0) {
            tasksTbl[i].sleepTick--;
        }

        if (tasksTbl[i].sleepTick == 0 &&
            tasksTbl[i].state == SLEEPING)
        {
            tasksTbl[i].state = READY;
        }
    }

    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PENDSVSET;

}

/*******************************************************************
*                               SysTick_Init()
*
* Configures SysTick timer to generate 1 ms interrupts.
*********************************************************************/
void SysTick_Init(void)
{
    uint32_t ui32SysClock = SysCtlClockGet();
    SysTickPeriodSet(ui32SysClock / 1000);              // 1ms period
    SysTickIntRegister(SysTick_Handler);
    SysTickIntEnable();
    SysTickEnable();
}



/*******************************************************************
*                               OS_Scheduler()
*
* TODO: Implement cooperative scheduling logic.
* Loops through tasks and executes those ready to run.
*********************************************************************/
void OS_Scheduler()
{
    if (currentTask != NULL && currentTask->state == RUNNING) {
        currentTask->state = READY;
    }
    int bestIdx  = -1;

    for (uint8_t i = 0; i < taskCount; i++)
    {
        if (tasksTbl[i].state == READY &&
            tasksTbl[i].sleeptick == 0)
        {
            bestIdx  = i;
            break;
        }
    }

    if (bestIdx >= 0) {
        currentTask = &tasksTbl[bestIdx];
        currentTask->state = RUNNING;
    } else {
        currentTask = &tasksTbl[0];
        currentTask->state = RUNNING;
    }
}

void OS_Yield(void)
{
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PENDSVSET;
}

void OS_Sleep(void)
{
    if (currentTask == NULL) return;
    currentTask->sleepTick = currentTask->sleepPeriod;
    currentTask->state = SLEEPING;
    OS_Yield();
}

