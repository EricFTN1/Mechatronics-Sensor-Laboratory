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
#define COLLECTOR    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4))) //PF4

// PortE masks
#define BROWN_HALL_MASK  2 //PE1
#define BLUE_HALL_MASK   4 //PE2
#define ORANGE_HALL_MASK 8 //PE3

//PORT E Band
#define BROWN_HALL    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4))) //PE1
#define BLUE_HALL     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4))) //PE2
#define ORANGE_HALL   (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4))) //PE3




void initPain();

#endif


