#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <serial.h>
#include <os.h>

// GLOBALS

#define NULL 0

int cur_butt  = -1;
int prev_butt = -1;

struct State {
  const struct State *Next[3];
  char output[3];
};

typedef const struct State STyp;

// INPUTS
#define p    0
#define SAME 1
#define DIFF 2

// OUTPUTS
#define blank 0
#define CB    1
#define CB2   2
#define CB3   3
#define CB4   4

// STATES
#define START &fsm[0]
#define P1    &fsm[1]
#define P2    &fsm[2]
#define P3    &fsm[3]
#define P4    &fsm[4]

STyp fsm[5] = {
  {{START, P1, P1}, {blank, blank, blank}},

  {{START, P2, P1}, {CB, blank, CB}},

  {{START, P3, P1}, {CB2, blank, CB2}},

  {{START, P4, P1}, {CB3, blank, CB3}},

  {{START, P1, P1}, {CB4, blank, CB4}},
};

int but21[4] = {'D', 'E', 'F', '3'};
int but31[4] = {'A', 'B', 'C', '2'};
int but41[4] = {'1', '1', '1', '1'};
int but22[4] = {'M', 'N', 'O', '6'};
int but32[4] = {'J', 'K', 'L', '5'};
int but42[4] = {'G', 'H', 'I', '4'};
int but23[4] = {'W', 'X', 'Y', '9'};
int but33[4] = {'T', 'U', 'V', '8'};
int but43[4] = {'P', 'R', 'S', '7'};
int but24[4] = {'#', '#', '#', '#'};

// rows and columns according to diagram
// in handout
// eg: row1 = A B C D
// eg: row2 = 3 6 9 #
int button_pressed(int i)
{
  int ret;

  if (i == 0) //row2
    ret = 20;
  if (i == 1) //row3
    ret = 30;
  if (i == 2) //row4
    ret = 40;

  if (!(GPIO_PORTD_DATA_R & 0x1)) // col1
    {
      ret += 1;
      os_delay_ms(50);
      while(!(GPIO_PORTD_DATA_R & 0x1)){}
      return ret;
    }
  if (!(GPIO_PORTD_DATA_R & 0x2)) // col2
    {
      ret += 2;
      os_delay_ms(50);
      while(!(GPIO_PORTD_DATA_R & 0x2)){}
      return ret;
    }
  if (!(GPIO_PORTD_DATA_R & 0x4)) // col2
    {
      ret += 3;
      os_delay_ms(50);
      while(!(GPIO_PORTD_DATA_R & 0x4)){}
      return ret;
    }
  if (!(GPIO_PORTD_DATA_R & 0x8)) // col2
    {
      ret += 4;
      os_delay_ms(50);
      while(!(GPIO_PORTD_DATA_R & 0x8))
	return ret;
    }

  return -1;
}

int rc_to_index(int rc)
{
  int ret;
  prev_butt = cur_butt;

  if (rc == -1) // No presses.
    ret =  p;

  else if (rc == prev_butt){ // Last 2 presses were the same.
    ret = SAME;
    cur_butt = rc;
  }

  else if (rc != prev_butt){
    ret = DIFF;
    cur_butt = rc;
  }

  return ret; // Last 2 presses were different.
}

int get_input()
{
  int ret = p;

  int j;
  for(j = 0; j < 10; j++) // 100ms * 10 = 1 second
    {
      int i;
      for(i = 0; i < 3; i++)
	{
	  // ignore row1
	  if (i == 0)
	    GPIO_PORTB_DATA_R = 0x0D; // Turn on row2
	  if (i == 1)
	    GPIO_PORTB_DATA_R = 0x0B; // Turn on row3
	  if (i == 2)
	    GPIO_PORTB_DATA_R = 0x07; // Turn on row4

	  ret = rc_to_index(button_pressed(i));

	  if (ret != p)
	    return ret;
	}
      os_delay_ms(200);
    }
  return ret;
}

char get_char(int idx)
{
  // int but21[4] = ['D', 'E', 'F', '3']
  // int but31[4] = ['A', 'B', 'C', '2']
  // int but41[4] = ['1', '1', '1', '1']
  // int but22[4] = ['M', 'N', 'O', '6']
  // int but32[4] = ['J', 'K', 'L', '5']
  // int but42[4] = ['G', 'H', 'I', '4']
  // int but23[4] = ['W', 'X', 'Y', '9']
  // int but33[4] = ['T', 'U', 'V', '8']
  // int but43[4] = ['P', 'R', 'S', '7']
  // int but24[4] = ['#', '#', '#', '#']

  if (prev_butt == 21) return but21[idx];
  if (prev_butt == 31) return but31[idx];
  if (prev_butt == 41) return but41[idx];
  if (prev_butt == 22) return but22[idx];
  if (prev_butt == 32) return but32[idx];
  if (prev_butt == 42) return but42[idx];
  if (prev_butt == 23) return but23[idx];
  if (prev_butt == 33) return but33[idx];
  if (prev_butt == 43) return but43[idx];
  if (prev_butt == 24) return but24[idx];

  return NULL; // unsupported button
}


char get_output(int out)
{
  char ret = NULL;

  if (out == blank)
    ret = NULL;

  if (out == CB)
    ret = get_char(0);

  if (out == CB2)
    ret = get_char(1);

  if (out == CB3)
    ret = get_char(2);

  if (out == CB4)
    ret = get_char(3);

  return ret;
}

void ProgA(void) {
  STyp *pt;
  int input;
  int output;
  pt = START;
  
  while(1)
    {
      input = get_input();
      output = get_output(pt->output[input]);
      pt = pt->Next[input];

      if (output != NULL)
	SerialWrite(&output);
    }
}
