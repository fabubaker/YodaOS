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
#include <serial.h>

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
        {&TCBs[3],&TCBs[2].R0,2,{0},0,0,0,0,0,0,0                ,0x61000000},
        {&TCBs[2],&TCBs[1].R0,1,{0},0,0,0,0,0,0,0                ,0x61000000},
        {&TCBs[0],&TCBs[3].R0,3,{0},0,0,0,0,0,0,0                ,0x61000000}
};


#define MAX_PREEMPT 4     // Including non_preempt_block
#define MAX_NON_PREEMPT 3

// non-preemptive functions' context
void* non_preempt_tasks[MAX_NON_PREEMPT]; 
int   non_preempt_times[MAX_NON_PREEMPT];
uint64_t non_preempt_last_run[MAX_NON_PREEMPT] = {0};

volatile unsigned int preempt_count = 1; // Since non_preempt is the first
volatile unsigned int non_preempt_count = 0;

volatile struct TCB* RunPtr = &TCBs[0];
volatile int* StackPtr = 0;
volatile char bootstrapping;
volatile uint64_t curr_time = 0;

/*
 * @brief   Kickstarts the OS.
 * @return  1 if successful, 0 if failed
 */
int os_start()
{ 
  // initialize timers and ports.
  hw_init();

  // init context switching timer.
  Timer5AInit();

  // init Systick for global timer.
  SystickInit();

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
 * @brief Delay functionality provided by OS.
 *        Uses Systick Timer for millisecond delay.
 * @param delay - Milliseconds to delay for. 
 */
void os_delay_ms(uint64_t delay)
{
	uint64_t start_time = curr_time;
	while(((curr_time - start_time) >> 22) < delay);
}

/* @brief Locks the global semaphore 
 *
 * @param flag to toggle
 *
 * @retval 1 if flag acquired
 *         0 if failed
 */
int  os_lock(int flag)
{
	int retval = 0;
	
	IntDisable(INT_TIMER5A);
	if(flag) // lock is free
	{
		flag   =  0;
		retval =  1;
	}
	else
		retval = 0;

	IntEnable(INT_TIMER5A);
	return retval;
}

/* Unlocks the global semaphore */
void os_unlock(int flag)
{
	flag = 1;
}

/* 
 * The hw_init function is run before the OS 
 * is started. Use it to initialize hardware such as
 * ports, timers, etc.
 *
 * For user to define appropriately.
 */
void hw_init()
{
  /* Port A is reserved for serial writer */

  // Activate clock for the port.
  SYSCTL_RCGCGPIO_R |= 0x2A; // enable clock for PORT B,D,F.

  // Unlock the pin.
  GPIO_PORTB_LOCK_R = 0x4C4F434B;
  GPIO_PORTD_LOCK_R = 0x4C4F434B;   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;

  // Set commit register.
  GPIO_PORTB_CR_R = 0xFF;
  GPIO_PORTD_CR_R = 0xFF;
  GPIO_PORTF_CR_R = 0xFF;

  // Set direction for ports.
  /* GPIO_PORTB_DIR_R = 0x0F;    // Port B0-3 are output. */
  /* GPIO_PORTD_DIR_R = 0x00;    // Port D0-3 are input. */
  /* GPIO_PORTF_DIR_R = 0x00;    // Port  */

  GPIO_PORTB_DEN_R = 0xFF;
  GPIO_PORTD_DEN_R = 0xFF;
  GPIO_PORTF_DEN_R = 0xFF;

  SetupSerial();
}

/*
 * @brief Increments curr_time according to
 *        Systick counter.
 */
void SystickHandler()
{
	curr_time += 900719925;	
}

void SystickInit()
{
    NVIC_ST_RELOAD_R = 0x00FFFFFF;
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x7; // Interrupts with 80Mhz clock.
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
 * @brief    Contains block of code that runs non-preemptive tasks.
 */
void non_preempt_block()
{
    while(1)
    {
        int i = 0;
        for (; i < non_preempt_count; i++)
        {
	  if (((curr_time - non_preempt_last_run[i]) >> 32) >= non_preempt_times[i])
	  {
		  non_preempt_last_run[i] = curr_time;
		  void (*task)(void) = non_preempt_tasks[i];
		  (*task)(); // Run the task till completion.
	  }
        }
    }
}

