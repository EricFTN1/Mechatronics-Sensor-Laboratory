#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_
#include <stdint.h>

#define sf 6
#define MAX_MOTOR_SPEED 1023

extern int32_t desired_pwm;
extern uint16_t current_speed;

void initMotor();
void motorSpeedFw(int ms0);
void motorSpeedBw(int ms0);
void motorSpeedCcw(int ms0);
void motorSpeedCw(int ms0);
void motorSpeedZero();
uint16_t motorSpeedPlus();
uint16_t  motorSpeedMinus();

#endif
