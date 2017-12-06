#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <serial.h>
#include <os.h>

struct LEDState {
  const struct LEDState *Next[2];
  char output;
};

typedef const struct LEDState LEDSTyp;

// INPUTS
#define null  0
#define press 1

// OUTPUTS
#define blank 0
#define LED1  1
#define LED2  2
#define LED3  3
#define LED4  4 

// STATES
LEDSTyp LEDfsm_1[5] = {
  {{&LEDfsm_1[0], &LEDfsm_1[1]}, blank}, 
  {{&LEDfsm_1[2], &LEDfsm_1[0]}, LED1},
  {{&LEDfsm_1[3], &LEDfsm_1[0]}, LED2},
  {{&LEDfsm_1[4], &LEDfsm_1[0]}, LED3},
  {{&LEDfsm_1[1], &LEDfsm_1[0]}, LED4}
};

void ProgB()
{
       
	LEDSTyp *pt;
	int input;
	int output;
	pt = &LEDfsm_1[0];

	// Ideally should couple
	// this with semaphore.
	int priority = 0;
	
	while(1)
	{
	  input = !(GPIO_PORTF_DATA_R & 0x1);
	  os_delay_ms(20);
	  pt    = pt->Next[input];

	  if (pt == &LEDfsm_1[0]) // No blink
	  {
		  if (priority)
		  {
			  // Unlock semaphore
			  os_unlock();
			  priority = 0;
			  GPIO_PORTE_DATA_R = 0x0;
		  }			 		      
	  }

	  if (pt == &LEDfsm_1[1]) // LED1
	  {
		  if (!priority)
		  {
			  // Lock semaphore
			  while(!os_lock());
			  priority = 1;
		  }
		  
		  GPIO_PORTE_DATA_R = 0x2;
	  }

	  if (pt == &LEDfsm_1[2]) // LED2
	  {
		  GPIO_PORTE_DATA_R = 0x4;
	  }

	  if (pt == &LEDfsm_1[3]) // LED3
	  {
		  GPIO_PORTE_DATA_R = 0x8;
	  }

	  if (pt == &LEDfsm_1[4]) // LED4
	  {
		  GPIO_PORTE_DATA_R = 0x10;
	  }

	  os_delay_ms(50);
	}
}
