// PIN LIST:

// Analog input = PE0 (P30, siltscreen typo)
// PWM = PIN PB6
// Photoresistor input = PC6, goes to wide timer.
// Everything (should) work but for some reason interrupts are not working as they should


#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "wait.h"
#include <stddef.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "pain.h"

#define PE1_PIN         (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*1)))  //PE1
#define PUSH_BUTTON     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))  //PF4
#define COMP0_NEG_INPUT (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*7))) //PC7

#define FREQ_IN_MASK   64     //PC6
#define COMP0_NEG_MASK 128    //PC7
#define PE1_MASK       2        //PE1 Input to charge and discharge circuit
#define PUSH_BUTTON_MASK 16  //PF4

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

bool timeMode = false;
uint64_t frequency = 0;
uint32_t time = 0;
int test = 0;
uint32_t timeToFl= 0;

    //-----------------------------------------------------------------------------
    // Subroutines
    //-----------------------------------------------------------------------------
void enableCounterMode()
{
    // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 1 Hz interrupt rate
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);              // turn-on interrupt 37 (TIMER1A)

    // Configure Wide Timer 1 as counter of external events on CCP0 pin
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_POS;           // count positive edges
    WTIMER1_IMR_R = 0;                               // turn-off interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
}

void disableCounterMode()
{
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off time base timer
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off event counter
    NVIC_DIS0_R = 1 << (INT_TIMER1A-16);             // turn-off interrupt 37 (TIMER1A)
}

void disableTimerMode()
{
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter
    NVIC_DIS3_R = 1 << (INT_WTIMER1A-16-96);         // turn-off interrupt 112 (WTIMER1A)
}

// Frequency counter service publishing latest frequency measurements every second
void timer1Isr()
{
    frequency = WTIMER1_TAV_R;                   // read counter input
    WTIMER1_TAV_R = 0;                           // reset counter for next period
    //GREEN_LED ^= 1;                              // status
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;           // clear interrupt flag
}

// Period timer service publishing latest time measurements every positive edge
void wideTimer1Isr()
{
    time = WTIMER1_TAV_R;                        // read counter input
    WTIMER1_TAV_R = 0;                           // zero counter for next edge
    GREEN_LED ^= 1;                              // status
    WTIMER1_ICR_R = TIMER_ICR_CAECINT;           // clear interrupt flag
}

// Initialize Hardware
void initFreq()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | BLUE_LED_MASK;  // bits 1 and 2 are outputs, other pins are inputs
    GPIO_PORTF_DIR_R &= ~PUSH_BUTTON_MASK;               // bit 4 is an input
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | BLUE_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= PUSH_BUTTON_MASK | GREEN_LED_MASK | BLUE_LED_MASK;
                                                         // enable LEDs and pushbuttons
    GPIO_PORTF_PUR_R |= PUSH_BUTTON_MASK;                // enable internal pull-up for push button

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTC_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC6_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_WT1CCP0;
    GPIO_PORTC_DEN_R |= FREQ_IN_MASK;                // enable bit 6 for digital input

    initPain();
}

void initComparator(){
    SYSCTL_RCGCACMP_R |= SYSCTL_RCGCACMP_R0;
    _delay_cycles(3);
    // Configure COMP0_NEG_MASK as an analog input
    GPIO_PORTC_AFSEL_R |=  COMP0_NEG_MASK;                // select alternative functions for PC7
    GPIO_PORTC_DEN_R   &= ~COMP0_NEG_MASK;              // turn off digital operation
    GPIO_PORTC_AMSEL_R |= COMP0_NEG_MASK;                 // turn on analog operation on pin PC7
    GPIO_PORTC_DIR_R   &= ~COMP0_NEG_MASK;

    COMP_ACREFCTL_R |= COMP_ACREFCTL_VREF_M | COMP_ACREFCTL_EN;
    COMP_ACCTL0_R   |= COMP_ACCTL0_ASRCP_REF;   //| COMP_ACCTL0_ISLVAL | COMP_ACCTL0_CINV;
}

void enableTimerMode()
{
    // Configure Wide Timer 1 as counter
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_TAILR_R = 0xFFFFFFFF;
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;               // turn-on counter
}

void initTimer()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    _delay_cycles(3);
    enableCounterMode();
}

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R4 | SYSCTL_RCGCGPIO_R5;  //C D E
    _delay_cycles(3);

    GPIO_PORTE_DIR_R |= PE1_MASK;              //PE1 is output to charge and discharge Capacitor
    GPIO_PORTE_DEN_R |= PE1_MASK;

    // Initialize system clock to 40 MHz

    initUart0();
    initTimer();
    initComparator();
    initFreq();
    setUart0BaudRate(115200,40000000);

}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{

    // Initialize hardware
    initHw();

    putsUart0("Mechatronics Lab 5\n");

    // Configure selected mode

    char str[40];
    int history[5];
    int iter = 0;

    // Configure selected mode
    if (timeMode)
    {
        disableCounterMode();
        enableTimerMode();
    }
    else
    {
        disableTimerMode();
        enableCounterMode();
    }

    while (true)
    {
        if (!PUSH_BUTTON)
        {
            timeMode = !timeMode;
            if (timeMode)
            {
                disableCounterMode();
                enableTimerMode();
                snprintf(str, sizeof(str),"Cap Mode\n");
                putsUart0(str);
            }
            else
            {
                disableTimerMode();
                enableCounterMode();
                snprintf(str, sizeof(str),"Metal Mode\n");
                putsUart0(str);
            }
        }

        if (timeMode == 1)
        {
           PE1_PIN = 1;                   //discharge cap
           test = COMP_ACSTAT0_R;

           waitMicrosecond(10);
           PE1_PIN = 0;
           WTIMER1_TAV_R = 0;         //reset timer

           test = COMP_ACSTAT0_R;

           while((COMP_ACSTAT0_R & 0x00000002));  //means V+ > V-

           time = WTIMER1_TAV_R;
           WTIMER1_TAV_R = 0;

           timeToFl = (time-483)/0.6423;  //time to oz to ml
//           if (time < 580)
//                   {
//                       timeToFl = 0;
//                   }
           waitMicrosecond(1000000);
           snprintf(str, sizeof(str), "Time:    %7"PRIu32" (ticks)", time);
           putsUart0(str);

           snprintf(str, sizeof(str), "mL:    %7"PRIu32" (mL)\n", timeToFl);
           putsUart0(str);
        }

        if (timeMode == 0)
        {
            history[iter] = frequency;
            uint32_t averagef = 0;
            int i = 0;
            for(i = 0; i < 5; i++){
                averagef += history[i];
            }
            averagef /= 5;



            iter++;
            if(iter > 4){
                iter = 0;
                snprintf(str, sizeof(str), "Frequency: %7"PRIu32" (Hz)\n", averagef);
                putsUart0(str);
            }

            if ( (averagef > 94000) && (averagef < 96000) )
            {
                RED_LED = 1;  //bottle
            }
            else if ( ( averagef > 97000) && (averagef < 100000) )
            {
                BLUE_LED = 1; //PSU
            }
            else if ( (averagef > 125000) && (averagef < 140000) )
            {
                GREEN_LED = 1; //CHAIR
            }
            else
            {
                RED_LED = 0;
                BLUE_LED = 0;
                GREEN_LED = 0;
            }
            waitMicrosecond(1000000);
        }
    }
}


