#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <os.h>
#include <tasks.h>

int main(void) {

  os_register_preempt(ProgA);
  os_register_preempt(ProgB);
  os_register_preempt(ProgC);

  os_register_nonpreempt(&blink1, 1);
  os_register_nonpreempt(&blink3, 3);
  os_register_nonpreempt(&blink5, 5);

  os_init();
  
  while(1);
  
}










