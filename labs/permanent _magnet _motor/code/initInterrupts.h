#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stdbool.h>

#define TRIG         (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4))) //PE2

//PORT D masks
#define IR_OUT_MASK     8   //PD3
#define ECHO_MASK       64  //PC6

//PORT F Masks
#define PT_OUT1_MASK   4    //PF2
#define PT_OUT2_MASK  16    //PF4

extern uint8_t phase;
extern int32_t DistFromWall;

void initInterrupts(void);
extern void initInterrupts(void);

void PortFISR(void);
extern void PortFISR(void);

void oneshotISR1(void);
extern void oneshotISR1(void);

void oneshotISR2(void);
extern void oneshotISR2(void);

void wideTimer3BIsr(void);
extern void wideTimer3BIsr(void);

void PortDISR(void);
extern void PortDISR(void);

void PortAISR(void);
extern void PortAISR(void);

extern void reset(void);
extern void resetKey(void);




extern void reset_distance(void);
extern void distSet(int moveAmount);
extern int32_t speed;
extern int32_t dist;
extern int32_t traveled;
extern int8_t speedmax;
extern int16_t precision;
extern int32_t dist_left;
extern int32_t dist_right;
extern int16_t pi;          //3.145 *10
extern int16_t angle;
extern int32_t DistFromWall;
extern uint8_t phase;

extern uint32_t key;
extern int8_t state;
extern bool flag;

#endif // INTERRUPTS_H
