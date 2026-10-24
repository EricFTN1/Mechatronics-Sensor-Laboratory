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


    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    _delay_cycles(3);
    GPIO_PORTF_CR_R |= 1;
    _delay_cycles(3);
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;


    GPIO_PORTF_DIR_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK;
    GPIO_PORTF_DEN_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK | COLLECTOR_MASK;

    GPIO_PORTF_DIR_R &= ~RED_LED_MASK;  //PF1 Collector (using the RED_LED pin as I'm not using it otherwise.

    RED_LED = 0;
    BLUE_LED = 0;
    GREEN_LED = 0;

    GPIO_PORTF_DEN_R |= 1 | 16;
    GPIO_PORTF_DIR_R &= ~(1 | 16);
    GPIO_PORTF_PUR_R |= 1 | 16;
    GPIO_PORTF_PUR_R |= 1 | 16;
    GPIO_PORTF_IS_R &= ~(1 | 16); // Enable edge interrupts
    GPIO_PORTF_IBE_R |= (1 | 16); // Set IEV as determinant
    GPIO_PORTF_ICR_R |= (1 | 16); // Clear Interrupts
    NVIC_EN0_R = 1 << (INT_GPIOF-16);                // turn-on interrupt(GPIOF)
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000;
}

void portFGpioInterrupt(){
    if(SW1){
        GPIO_PORTF_ICR_R |= 16;
        GREEN_LED ^= 1;
        void motorSpeedPlus();
    }
    if(SW2){
        GPIO_PORTF_ICR_R |= 1;
        BLUE_LED ^= 1;
        void motorSpeedMinus();
    }
}



