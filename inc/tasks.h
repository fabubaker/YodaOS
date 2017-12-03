/* @file tasks.h
 *
 * Header file for user-defined tasks.
 * Declare and define preemptive / non-preemptive functions below
 * to be run by the OS.
 */

#ifndef TASKS_H_
#define TASKS_H_

/*
 * Premptive tasks are declared below. 
 * User defines these tasks elsewhere.
 */

void ProgA();
void ProgB();
void ProgC();

/* 
 * Non-preemptive tasks are declared below.
 * User defines these tasks elsewhere.
 */

void blink1();
void blink3();
void blink5();


#endif
