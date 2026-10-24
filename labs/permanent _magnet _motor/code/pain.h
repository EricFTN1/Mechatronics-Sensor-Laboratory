/*
 * pain.h
 *
 *  Created on: Mar 24, 2024
 *      Author: clara
 */


#ifndef PAIN_H_
#define PAIN_H_
#include <stdint.h>

#define RED_LED_MASK 2      //PF1
#define BLUE_LED_MASK 4     //PF2
#define GREEN_LED_MASK 8    //PF3
#define COLLECTOR_MASK 16   //PF4

#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4))) //PF1
#define BLUE_LED     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4))) //PF2
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4))) //PF3
#define COLLECTOR    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4))) //PF4

#define SW2  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 0*4))) //F0 decrease
#define SW1  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4))) //F4 increase

void portFGpioInterrupt();


void initPain();

#endif


