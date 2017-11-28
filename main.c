#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <os.h>
#include <tasks.h>

int main(void) {

  os_init();

  os_register_preempt(&ProgA);
  os_register_preempt(&ProgB);
  os_register_preempt(&ProgC);

  while(1);
  
}










