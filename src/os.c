/*
 * os.c
 *
 * @brief Contains core os functions for context switching
 *        and mutual exclusion.
 *
 * @compat Uses Timer5A and SysTick timer for handling 
 *         systems-critical code. 
 */
#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <os.h>
#include <tasks.h>

struct TCB
{
    struct TCB *next;
    unsigned int* StactPt;
    int id;
    unsigned int ExtraStack[100];
    unsigned int R0;
    unsigned int R1;
    unsigned int R2;
    unsigned int R3;
    unsigned int R12;
    unsigned int LR;
    void (*InitPC)(void);
    unsigned int PCR;
};

void non_preempt_block();

// pre-emptive tasks
// TCBs[0] is reserved for the non-preemptive block.
struct TCB TCBs[4] = {
        {&TCBs[1],&TCBs[0].R0,0,{0},0,0,0,0,0,0,non_preempt_block,0x61000000},
        {&TCBs[2],&TCBs[1].R0,1,{0},0,0,0,0,0,0,0                ,0x61000000},
        {&TCBs[3],&TCBs[2].R0,2,{0},0,0,0,0,0,0,0                ,0x61000000},
        {&TCBs[0],&TCBs[3].R0,3,{0},0,0,0,0,0,0,0                ,0x61000000}
};


#define MAX_PREEMPT 4     // Including non_preempt_block
#define MAX_NON_PREEMPT 3

// address of non-preemptive functions
void* non_preempt_tasks[MAX_NON_PREEMPT]; 
int   non_preempt_times[MAX_NON_PREEMPT];

volatile unsigned int preempt_count = 1; // Since non_preempt is the first
volatile unsigned int non_preempt_count = 0;

volatile struct TCB* RunPtr = &TCBs[0];
volatile int* StackPtr = 0;
volatile char bootstrapping;

/*
 * @brief Uses Systick Timer as a delay.
 * @param delay - Seconds to delay for. 
 */
void delaySeconds(uint32_t delay)
{
	int i = 0;
	for (; i < delay; i++)
	{
	  NVIC_ST_RELOAD_R = 0x3D0900; // Delay for 1 second.
	  NVIC_ST_CURRENT_R = 0;
	  NVIC_ST_CTRL_R = 1;   // Use 16Mhz/4 PIOSC clock with no interrupts.
	  NVIC_ST_RELOAD_R = 0; // Disable wrap-around during context-switch.
	  while ((NVIC_ST_CTRL_R&0x00010000)==0){}
	}
}

/* 
 * @brief  Timer handler for context switching.
 */
void Timer5AHandler()
{
    TIMER5_ICR_R = 0x00000001;
    asm("   .global RunPtr");
    asm("StackAdd: .field RunPtr");

    if(bootstrapping == 1)
    {
        bootstrapping = 0;
        asm("  LDR R0,StackAdd ");
        asm("  LDR R0,[R0] ");
        asm("  LDR R13,[R0,#4]");
        return;
    }

    asm("  LDR R0,StackAdd");
    asm("  LDR R0,[R0] ");
    asm("  STR R13,[R0,#4]");
    RunPtr = RunPtr->next;
    asm("  LDR R0,StackAdd ");
    asm("  LDR R0,[R0] ");
    asm("  LDR R13,[R0,#4]");
}

/*
 * @brief  Initializes Timer5A for context switching.
 */
void Timer5AInit()
{
    SYSCTL_RCGCTIMER_R |= 0x20;
    TIMER5_CTL_R &= 0x00000000;
    TIMER5_CFG_R = 0x00000000;
    TIMER5_TAMR_R = 0x00000002;
    TIMER5_TAILR_R = 0x0000FFFF;
    TIMER5_TAPR_R = 0;
    TIMER5_IMR_R = 0x00000001;
    TIMER5_ICR_R = 0x00000001;
    NVIC_PRI5_R = (NVIC_PRI5_R & 0x00FFFFFF)|0x80000000;
    NVIC_EN0_R = 1<<23;
    TIMER5_CTL_R = 0x00000001;
    IntEnable(INT_TIMER5A);
}

/*
 * @brief   Initializes OS configuration.
 * @return  1 if successful, 0 if failed
 */
int os_init()
{ 
  // Activate clock for the port
  SYSCTL_RCGCGPIO_R |= 0x22; // enable clock for PORT F and PORTB

  Timer5AInit();
  bootstrapping = 1;

  return 1;
}

/* 
 * @brief       Registers preemptive task with OS.
 * @param task  Premptive Task to register with OS. 
 * @return      1 if successful, 0 if failed.
 */ 
int os_register_preempt(void (*task)(void))
{
  if (preempt_count >= MAX_PREEMPT)
      return 0; // At max limit    

  TCBs[preempt_count].InitPC = task;
  preempt_count += 1;
  return 1;
}

/*
 * @brief       Registers non-preemptive task with OS.
 * @param task  Non-Premptive Task to register with OS.
 * @param time  Run this task every 'time' seconds.
 * @return      1 if successful, 0 if failed.
 */
int os_register_nonpreempt(void (*task)(void), int time)
{
  if (non_preempt_count >= MAX_NON_PREEMPT)
    return 0; // At max limit

  non_preempt_tasks[non_preempt_count] = task;
  non_preempt_times[non_preempt_count]  = time;
  non_preempt_count += 1;
  return 1;
}

/*
 * @brief    Contains block of code that runs non-preemptive tasks.
 */
void non_preempt_block()
{
    while(1)
    {
        int i = 0;
        for (; i < non_preempt_count; i++)
	    {
            delaySeconds(non_preempt_times[i]);
            void (*task)(void) = non_preempt_tasks[i];
            (*task)(); // Run the task till completion.
	    }
    }
}
