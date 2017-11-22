/*
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>

/* Preemptive tasks below */

void ProgA(void);
void ProgB(void);
void ProgC(void);

struct TCB
{
    struct TCB *next;
    unsigned int* StactPt;
    int id;
    unsigned int ExtraStack[50];
    unsigned int R0;
    unsigned int R1;
    unsigned int R2;
    unsigned int R3;
    unsigned int R12;
    unsigned int LR;
    void (*InitPC)(void);
    unsigned int PCR;
};

extern struct TCB TCBs[3] = {
        {&TCBs[1],&TCBs[0].R0,1,{0},0,0,0,0,0,0,ProgA,0x61000000},
        {&TCBs[2],&TCBs[1].R0,1,{0},0,0,0,0,0,0,ProgB,0x61000000},
        {&TCBs[0],&TCBs[2].R0,1,{0},0,0,0,0,0,0,ProgC,0x61000000}
};

unsigned int static Timervalue = 0;
unsigned int static old = 0;
unsigned int static done = 0;
extern volatile struct TCB* RunPtr = &TCBs[0];
extern volatile int* StackPtr=0;
volatile char bootstrapping;

void Timer2AHandler()
{
    TIMER2_ICR_R = 0x00000001;
    asm("   .global RunPtr");
    asm("StackAdd: .field RunPtr");

    if(bootstrapping == 1)
    {
        bootstrapping = 0;
        asm("  LDR R0,StackAdd ");
        asm("  LDR R0,[R0] ");
        asm("  LDR R13,[R0,#4]");
    }
    else
    {
        asm("  LDR R0,StackAdd");
        asm("  LDR R0,[R0] ");
        asm("  STR R13,[R0,#4]");
        RunPtr = RunPtr->next;
        asm("  LDR R0,StackAdd ");
        asm("  LDR R0,[R0] ");
        asm("  LDR R13,[R0,#4]");
    }

}

void Timer2AInit()
{
    SYSCTL_RCGCTIMER_R |= 0x04;
    TIMER2_CTL_R &= 0x00000000;    
    TIMER2_CFG_R = 0x00000000;
    TIMER2_TAMR_R = 0x00000002;
    TIMER2_TAILR_R = 0x0000FFFF;
    TIMER2_TAPR_R = 0;
    TIMER2_IMR_R = 0x00000001;
    TIMER2_ICR_R = 0x00000001;
    NVIC_PRI5_R = (NVIC_PRI5_R & 0x00FFFFFF)|0x80000000;
    NVIC_EN0_R = 1<<23;
    TIMER2_CTL_R = 0x00000001;
    IntEnable(INT_TIMER2A);
}

int main(void) {

    init();

    // step 1: Activate clock for the port
    SYSCTL_RCGCGPIO_R |= 0x22; // enable clock for PORT F and PORTB

    Timer2AInit();
    bootstrapping = 1;
    while(1);
    //return 0;
}










