// initTimers Example
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
#include "initTimers.h"
#include "tm4c123gh6pm.h"
#include "initInterrupts.h"
#include "wait.h"
#include "main.h"
int trigCount= 0;
void initTimers(void)
{

    //enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1 | SYSCTL_RCGCTIMER_R2 | SYSCTL_RCGCTIMER_R3; //reg timer
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R3 | SYSCTL_RCGCWTIMER_R1;    //wide timers
    _delay_cycles(3);

    // Configure Timer PF2 T1CCP0,
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;          // configure for oneshot mode (count down)
    TIMER1_TAILR_R = 1000000;                        // set load value to 1e5 for 40 Hz interrupt rate, 25ms
    TIMER1_IMR_R = TIMER_IMR_TATOIM;
    //TIMER1_CTL_R |= TIMER_CTL_TAEN;                // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);              // turn-on interrupt 37 (TIMER1A)

    // Configure Timer PF4 T2CPP0
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;          // configure for oneshot mode (count down)
    TIMER2_TAILR_R = 1000000;                         // set load value to 1e5 for 40 Hz interrupt rate, 25ms
    TIMER2_IMR_R = TIMER_IMR_TATOIM;
    //TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER2A-16);              // turn-on interrupt 39 (TIMER2A)

//    //PORT B
//    GPIO_PORTB_DIR_R &= ~TIME_TRIG_MASK;            //bit 3 and 4 are inputs
//    GPIO_PORTB_DEN_R |= TIME_TRIG_MASK ;              //enable pin
//    GPIO_PORTB_PDR_R |= TIME_TRIG_MASK ;              // enable internal pull-down for SW1

    // Configure Timer PB2 TC3CPP0 to trigger TRIG on PE2
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER3_TAILR_R = 4000000;                        // set load value to 40e5 for 10 Hz interrupt rate
    TIMER3_IMR_R = TIMER_IMR_TATOIM;
    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                // turn-on timer
    NVIC_EN1_R = 1 << (INT_TIMER3A-16-32);              // turn-on interrupt 51 (TIMER3A)

    // Configure WTimer PD3=IR_OUT
    GPIO_PORTD_AFSEL_R |= IR_OUT_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTD_PCTL_R  &= ~GPIO_PCTL_PD3_M;         // map alt fns to SIGNAL_IN
    GPIO_PORTD_PCTL_R  |= GPIO_PCTL_PD3_WT3CCP1;
    GPIO_PORTD_DEN_R   |= IR_OUT_MASK ;             // enable bit 6 for digital input

    WTIMER3_CTL_R &= ~TIMER_CTL_TBEN;                // turn-off counter before reconfiguring
    WTIMER3_CFG_R = 4;                               // configure as 32-bit counter (B only)
    WTIMER3_TBMR_R = TIMER_TBMR_TBCMR | TIMER_TBMR_TBCDIR; // configure for edge count mode, count up
    WTIMER3_CTL_R = TIMER_CTL_TBEVENT_NEG;           // count positive edges
    WTIMER3_IMR_R = 0;                               // turn-off interrupts
    WTIMER3_TBV_R = 0;                               // zero counter for first period
    WTIMER3_CTL_R |= TIMER_CTL_TBEN;                 // turn-on counter
    //NVIC_EN3_R = 1 << (INT_WTIMER3B - 16 - (32*3));  //vector WTIMER3B 117

    // Configure WTimer PC6,WT1CPP0=Echo PE2trig
    GPIO_PORTC_AFSEL_R |= ECHO_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R  &= ~GPIO_PCTL_PC6_M;         // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R  |= GPIO_PCTL_PC6_WT1CCP0;
    GPIO_PORTC_DEN_R   |= ECHO_MASK ;             // enable bit 6 for digital input

    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TBMR_R = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_BOTH;           // count both edges
    WTIMER1_IMR_R = TIMER_IMR_CAEIM;                               // turn-off interrupts
    WTIMER1_TBV_R = 0;                               // zero counter for first period
    //WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN3_R = 1 << (INT_WTIMER1A - 16 - (32*3));   //vector WTIMER1A 112


}

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void timer3ISR(void)
{
    WTIMER1_TAV_R = 0;
    if (phase != 2)
    {
        DistFromWall = -1;
    }
    TRIG = 1;
    waitMicrosecond(10);
    TRIG = 0;
    trigCount= WTIMER1_TAV_R;
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;
    GREEN_LED = 1;
    //WTIMER5_ICR_R = TIMER_ICR_CAECINT;
    TIMER3_IMR_R &= ~TIMER_IMR_TATOIM;
    //TIMER3_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER3_ICR_R |= TIMER_ICR_CAECINT;      //clears timer3ISR
}

void EchoISR(void) //PORT C6
{
    if (phase == 0)
    {
        WTIMER1_TAV_R = 0;
        phase = 1;
    }
    else
    {
        DistFromWall = WTIMER1_TAV_R;
        GREEN_LED = 0;
        phase = 2;
    }

    WTIMER1_ICR_R = TIMER_ICR_CAECINT;
}
