#include <tasks.h>

volatile int value = 0;
volatile int dummy = 0;

void ProgB()
{
  while(1)
    {
      value = value - 1;
      int i,j;
      //delay
      for( i = 0; i <150; i++)
	for( j = 0; j <1000;j++)
	  dummy = 2;
    }

}
