#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

// PortC and D masks
#define PC5_MOTOR_MASK 32
#define PC4_MOTOR_MASK 16

#define PD0_MOTOR_MASK  1
#define PD1_MOTOR_MASK  2

void move(int32_t leftf, int32_t leftr, int32_t rightf,int32_t rightr);
extern void move(int32_t leftf, int32_t leftr, int32_t rightf,int32_t rightr);

void initMotors(void);
extern void initMotors(void);

extern int32_t disparity;

#endif // MOTORS_H
