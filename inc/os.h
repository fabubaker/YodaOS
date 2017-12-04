/*
 * os.h
 */

#ifndef OS_H_
#define OS_H_

/* OS control */
int  os_start();
int  os_register_preempt();
int  os_register_nonpreempt();
void os_delay_ms(uint64_t delay);

/* Semaphore functions */
// flag == 1 means resource is free to use.
static int flag = 1; 

int  os_lock  (int flag);
void os_unlock(int flag);

/* User defined HW initializer */
void hw_init();
#endif 
