#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <tasks.h>

void hw_init()
{
  // Activate clock for the port
  SYSCTL_RCGCGPIO_R |= 0x22; // enable clock for PORT F and PORTB
}
