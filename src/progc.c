#include <tasks.h>

volatile int value = 0;
volatile int dummy = 0;

int Factorial(int n)
{
  if (n == 1)
    return 1;
  return n * Factorial(n-1);
}
void ProgC()
{
  while(1)
    {
      Factorial(10);
    }
}

