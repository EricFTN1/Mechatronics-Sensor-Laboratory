// initMotors Example
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
#include "initMotors.h"
#include "tm4c123gh6pm.h"

int32_t disparity = 10;

void initMotors(void)
{

    //SYSCTL_RCGCGPIO_R =  SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R3; //PORT C,D
    //_delay_cycles(3);

    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1 | SYSCTL_RCGCPWM_R0; //PWM M1,M0

    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM0 module

    SYSCTL_SRPWM_R = 0;// leave reset state

    PWM0_3_CTL_R = 0;            // turn-off PWM0 generator 3 (drives outs 6 and 7)
    PWM1_0_CTL_R = 0;            // turn-off PWM1 generator 0 (drives outs 0 and 1)

    PWM0_3_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO;
                                                     // output 6 on PWM0, gen3 , cmpa
    PWM0_3_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO;
                                                     // output 7 on PWM0, gen3 , cmpb
    PWM1_0_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
                                                     // output 0 on PWM1, gen0 , cmpa
    PWM1_0_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                     // output 1 on PWM1, gen0 , cmpb

    PWM0_3_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
    PWM1_0_LOAD_R = 1024;                            // (internal counter counts down from load value to zero)

    PWM0_3_CMPA_R = 0;                               // PC5_MOTOR_MASK off(0=always low, 1023=always high)
    PWM0_3_CMPB_R = 0;                               // PC4_MOTOR_MASK off
    PWM1_0_CMPA_R = 0;                               // PD0_MOTOR_MASK off
    PWM1_0_CMPB_R = 0;                               // PD1_MOTOR_MASK off

    PWM0_3_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM0 generator 3
    PWM1_0_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 0
    PWM0_ENABLE_R = PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;
    PWM1_ENABLE_R = PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN; // enable outputs

    // Configure motors //PORTC, In4 on M0PWM7 (PC5), M0PWM3a && In3 on M0PWM6 (PC4), M0PWM3b
    GPIO_PORTC_DEN_R |= PC5_MOTOR_MASK | PC4_MOTOR_MASK;
    GPIO_PORTC_AFSEL_R |= PC5_MOTOR_MASK | PC4_MOTOR_MASK;
    GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC5_M | GPIO_PCTL_PC4_M);
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC5_M0PWM7 | GPIO_PCTL_PC4_M0PWM6 ;

    //PORTD, In2 on M1PWM0 (PD0), M1PWM0a && In1 on M1PWM1 (PD1), M1PWM0b
    GPIO_PORTD_DEN_R |= PD0_MOTOR_MASK | PD1_MOTOR_MASK;
    GPIO_PORTD_AFSEL_R |= PD0_MOTOR_MASK | PD1_MOTOR_MASK;
    GPIO_PORTD_PCTL_R &= ~( GPIO_PCTL_PD0_M | GPIO_PCTL_PD1_M);
    GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_M1PWM0 | GPIO_PCTL_PD1_M1PWM1;
}

void move(int32_t leftf, int32_t leftr, int32_t rightf,int32_t rightr)
{
    if (leftf>0)
    {
        leftf = (int)((leftf * 323.0 / 100) + 700); // leftf
    }
    if (leftr>0)
    {
        leftr = (int)((leftr * 323.0 / 100) + 700); // leftr
    }
    if (rightf>0)
    {
        rightf = (int)(((rightf - disparity) * 323.0 / 100) + 700); // rightf
    }
    if (rightr>0)
    {
        rightr = (int)(((rightr - disparity) * 323.0 / 100) + 700); // rightr
    }

    PWM0_3_CMPA_R = leftf; // leftf
    PWM0_3_CMPB_R = leftr; // leftr
    PWM1_0_CMPA_R = rightf - disparity; // rightf
    PWM1_0_CMPB_R = rightr - disparity; // rightr
}


