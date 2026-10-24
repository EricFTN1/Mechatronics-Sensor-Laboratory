/*
 * systemph.h
 *
 *  Created on: Sep 1, 2024
 *      Author: clara
 */
#ifndef SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>

extern uint32_t atoi(char * str1);


void yield();
void kbhit();
void pibool(bool on);
void sched(bool prio_on);
void preempt(bool on);
void pidof(const char name[]);
void pkill(char * process);
void kill(uint32_t pid);
void ps();
void ipcs();




#endif

