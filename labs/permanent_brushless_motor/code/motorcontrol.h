#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_
#include <stdint.h>

#define sf 6
#define MAX_MOTOR_SPEED 1023

extern int32_t desired_pwm;

void initMotor();
void motorSpeedFw(int ms0);
void motorSpeedBw(int ms0);
void motorSpeedCoilA(int ms0);
void motorSpeedCoilB(int ms0);
void motorSpeedCoilC(int ms0);
void motorSpeedZero();

#define PWMC5_MASK 32    //PC5
#define PWMB6_MASK 64    //PB6
#define PWMB7_MASK 128   //PB7

#endif
