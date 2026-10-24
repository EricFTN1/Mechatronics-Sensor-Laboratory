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
#include "adc0.h"
#include <stddef.h>
#include "clock.h"
#include <math.h>
#include "pain.h"
#include "shell.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "motorcontrol.h"



#define FOUR_PIN      (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4)))
#define FIVE_EN_PIN      (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*5)))
#define SIX_EN_PIN      (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*6)))
#define FOURPIN 16
#define FIVEPIN 32
#define SIXPIN 64
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

//extern int strcmp(char * str1, char * str2);

// Initialize Hardware

#define AIN3_MASK 1

// PortC masks
#define FREQ_IN_MASK 64

// PortF masks
#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 8
#define PUSH_BUTTON_MASK 16

    //-----------------------------------------------------------------------------
    // Global variables
    //-----------------------------------------------------------------------------

bool timeMode = true;
uint32_t RPM = 0;
uint32_t time = 0;
uint16_t raw = 0;
float inverseBEMF = 0;
float actualBEMF = 0;
char str[100];
float BEMFRPM = 0;

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

//    TIMER0_CTL_R &= ~(TIMER_CTL_TAEN);                 // turn-off timer before reconfiguring
//    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
//    TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;        // configure for periodic mode (count down)
//    TIMER0_TAILR_R = 40000000;                         // set load value to 40e5 for 40 Hz interrupt rate
//    TIMER0_IMR_R |= TIMER_IMR_TATOIM;                  // turn-on debounce interrupt
//    TIMER0_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
//    NVIC_EN0_R = 1 << (INT_TIMER0A-16);              // turn-on interrupt 38 (TIMER1B)



    // Configure Wide Timer 1 as counter of external events on CCP0 pin
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_NEG;           // count neg edges
    WTIMER1_IMR_R = 0;                               // turn-off interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
}


// Frequency counter service publishing latest frequency measurements every second
void timer1Isr()
{
    RPM = WTIMER1_TAV_R;                   // read counter input
    RPM = (RPM/32)*6;
    WTIMER1_TAV_R = 0;
//    putsUart0("RPM: ");// reset counter for next period
//    putiUart0(RPM);
//    putcUart0('\n');
    motorSpeedZero();
    waitMicrosecond(1000);
    int16_t raw = readAdc0Ss3();
    motorSpeedFw(current_speed);
    inverseBEMF = ((raw+0.5)/4096)*3.3;
    actualBEMF = 10 - (inverseBEMF*(47+10)/10);
    BEMFRPM = (actualBEMF/0.0058)-0.091;
    GREEN_LED ^= 1;                              // status
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;           // clear interrupt flag
}

//void timer2Isr()
//{
//
//    TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
//}

// Period timer service publishing latest time measurements every positive edge

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= (SYSCTL_RCGCTIMER_R4 | SYSCTL_RCGCTIMER_R3 | SYSCTL_RCGCTIMER_R2 | SYSCTL_RCGCTIMER_R1 | SYSCTL_RCGCTIMER_R0);
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Configure AIN3 as an analog input
    GPIO_PORTE_AFSEL_R |= AIN3_MASK;                 // select alternative functions for AIN3 (PE0)
    GPIO_PORTE_DEN_R &= ~AIN3_MASK;                  // turn off digital operation on pin PE0
    GPIO_PORTE_AMSEL_R |= AIN3_MASK;                 // turn on analog operation on pin PE0

    // Configure LED and pushbutton pins

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTC_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC6_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_WT1CCP0;
    GPIO_PORTC_DEN_R |= FREQ_IN_MASK;                // enable bit 6 for digital input

    // Initialize system clock to 40 MHz
    _delay_cycles(3);
    initUart0();
    initPain();
    initMotor();
    initAdc0Ss3();
    setAdc0Ss3Mux(3);
    setAdc0Ss3Log2AverageCount(2);


    setUart0BaudRate(115200,40000000); // Set baud rate to project spec
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    motorSpeedFw(500);

    putsUart0("Mechatronics Lab 3\n");

    enableCounterMode();

    // Use blue LED to show mode

    // Endless loop performing multiple tasks
    while (true)
    {
        snprintf(str, sizeof(str),"PWM%:%5u RPM:%5u InverseBEMF:%.5f ActualBEMF:%.5f BEMFRPM:%.5f",(current_speed/10),RPM,inverseBEMF,actualBEMF,BEMFRPM);
        //snprintf(str, sizeof(str),"PWM%:%5"PRIu32 " RPM:%5"PRIu32 " InverseBEMF:%5f ActualBEMF:%5f BEMFRPM:%5f",(current_speed/10), RPM,inverseBEMF,actualBEMF,BEMFRPM);
        putcUart0('\n');
        putsUart0(str);
        //32 slits
        // debouncing not implemented until keyboard.c example
        // for now, use a delay to allow unique key presses to be detected
        int i;
        for(i = 0; i < 10; i++){
            if(!SW1){
                //GPIO_PORTF_ICR_R |= 16;
                motorSpeedPlus();
            }
            if(!SW2){
                //GPIO_PORTF_ICR_R |= 1;
                motorSpeedMinus();
            }
            waitMicrosecond(100000);
        }
    }
}


