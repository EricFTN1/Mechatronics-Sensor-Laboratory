#ifndef Eric_H
#define Eric_H

#include <stdint.h>

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4))) //PF1
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4))) //PF3
#define SLEEP_AL     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4))) //PE3


// PORTA masks
#define PIR_OUT_MASK 128    //PA7

//PORTE masks
#define SLEEP_MASK 8        //PE3

// PortF masks
#define RED_LED_MASK   2    //PF1
#define GREEN_LED_MASK 8    //PF3

extern int32_t dist;
extern bool flag;
extern uint32_t key;
extern uint8_t crank;

#endif // Eric_H
