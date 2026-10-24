// initInterrupts Example
// Eric Nguyen

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:



//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "initInterrupts.h"
#include "wait.h"
#include <time.h>
#include "pain.h"
#include "uart0.h"

void initInterrupts()
{

    GPIO_PORTE_DIR_R &= ~(BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK);  //PORT E inputs, HALL Sensors
    GPIO_PORTE_DEN_R |= BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK;

//    GPIO_PORTE_IS_R  &= ~(BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK);   //edges
//    GPIO_PORTE_IBE_R &= ~(BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK);   //single edge
//    GPIO_PORTE_IEV_R |= BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK;    //rising edge
//    GPIO_PORTE_IM_R  |= BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK;    //allows for same port interrupts
//    GPIO_PORTE_ICR_R |= BROWN_HALL_MASK|BLUE_HALL_MASK|ORANGE_HALL_MASK;    //clears interrupts to call again
//    NVIC_EN0_R = 1 << (INT_GPIOE-16);   // turn-on interrupt (GPIOE)

}

void PortEISR(void)
{

    if (GPIO_PORTE_MIS_R & BROWN_HALL_MASK)    //checks value of 2rd bit of MIS
        {
            putsUart0("Brown \n");
            GPIO_PORTE_ICR_R  |= BROWN_HALL_MASK; //clear interrupt to use again
        }
    if (GPIO_PORTE_MIS_R & BLUE_HALL_MASK)    //checks value of 2rd bit of MIS
        {
            putsUart0("BLUE \n");
            GPIO_PORTE_ICR_R  |= BLUE_HALL_MASK; //clear interrupt to use again
        }
    if (GPIO_PORTE_MIS_R & ORANGE_HALL_MASK)    //checks value of 2rd bit of MIS
        {
            putsUart0("ORANGE \n");


            GPIO_PORTE_ICR_R  |= ORANGE_HALL_MASK; //clear interrupt to use again
        }
}

