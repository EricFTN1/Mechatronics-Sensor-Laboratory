/*
 * pain.c
 *
 *  Created on: Mar 24, 2024
 *      Author: clara
 */
#include "tm4c123gh6pm.h"
#include "pain.h"


void initPain(){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    GPIO_PORTF_DIR_R &= ~COLLECTOR_MASK;  //PF4 Collector
    GPIO_PORTF_DIR_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK;
    GPIO_PORTF_DEN_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK | COLLECTOR_MASK;

    RED_LED = 0;
    BLUE_LED = 0;
    GREEN_LED = 0;
}




