#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stdbool.h>

void initInterrupts();
extern void initInterrupts();

void PortEISR(void);
extern void PortEISR(void);

#define BROWN_HALL    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4)))
#define BLUE_HALL     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))
#define ORANGE_HALL   (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))

#define BROWN_HALL_MASK  2 //PE1
#define BLUE_HALL_MASK   4 //PE2
#define ORANGE_HALL_MASK 8 //PE3

#endif // INTERRUPTS_H
