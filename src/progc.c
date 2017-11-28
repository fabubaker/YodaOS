#include <tasks.h>

extern volatile int value;
extern volatile int dummy;

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

