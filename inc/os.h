/*
 * os.h
 */

#ifndef OS_H_
#define OS_H_

int os_start();
int os_register_preempt();
int os_register_nonpreempt();

void os_delay_ms(uint64_t delay);
#endif 
