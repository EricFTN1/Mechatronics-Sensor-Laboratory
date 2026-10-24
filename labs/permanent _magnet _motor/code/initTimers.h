#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

#define IR_OUT_MASK     8   //PD3
#define ECHO_MASK      64   //PD6
#define TRIG_MASK       4   //PE2
#define TIME_TRIG_MASK  4   //PB2

void initTimers(void);
extern void initTimers(void);

void timer3ISR(void);
extern void timer3ISR(void);

void EchoISR(void);
extern void EchoISR(void); //PORT C6

#endif // TIMERS_H
