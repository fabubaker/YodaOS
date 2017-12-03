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
void os_lock();
void os_unlock();

void hw_init();
#endif 
