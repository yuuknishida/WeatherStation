#ifndef APP_OS_H_
#define APP_OS_H_

#include <stdint.h>
#include <stdbool.h>

#define NVIC_INT_CTRL_PENDSVSET  (1U << 28)

#define MAX_TASKS       7
#define STACK_SIZE      128

typedef enum {
    READY, RUNNING, BLOCKED, SLEEPING
} state_t;

typedef struct task{
    uint32_t *stkPtr;
    uint8_t id;
    void (*taskFunc)(void);     // Points to task function
    uint16_t sleepTick;         // How long task will sleep for
    state_t state;              // Task Current state
} task_t;

extern uint32_t taskStk[MAX_TASKS][STACK_SIZE];
extern task_t tasksTbl[MAX_TASKS];
extern task_t *currentTask;
extern uint8_t taskCount;

void OS_Init();
void OS_Launch();
void OS_AddTask(void (*taskFunc)(void), uint16_t ticks);
void SysTick_Handler(void);
void SysTick_Init(void);
void OS_Scheduler();
void OS_Yield(void);
void OS_Sleep(void);


#endif /* APP_OS_H_ */
