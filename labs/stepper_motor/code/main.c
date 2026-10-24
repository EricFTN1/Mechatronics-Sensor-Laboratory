
// Serial Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clock.h"
#include "pain.h"
#include "shell.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "motorcontrol.h"
#include "wait.h"

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
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R |= (SYSCTL_RCGCGPIO_R2);
    _delay_cycles(3);
    initUart0();
    initPain();
    initMotor();

    FIVE_EN_PIN = 1;
    SIX_EN_PIN = 1;

    setUart0BaudRate(115200,40000000); // Set baud rate to project spec
    // Configure LED pins
    GPIO_PORTC_DIR_R |= FOURPIN | FIVEPIN | SIXPIN;

    GPIO_PORTC_DEN_R |= FOURPIN | FIVEPIN | SIXPIN;
    FIVE_EN_PIN = 1;
    SIX_EN_PIN = 1;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

// Green Blue - Coil A - Purple
// Red PC4 - Coil B - Green

// White (Blue led) +
// Green (Red led) +
// Yellow (PC4 led)-
// Black (Green led) -

int main(void)
{
    initHw();
//    waitMicrosecond(100000);// for first part
//
//    motorSpeedFw(512);
//    motorSpeedBw(512);

    GREEN_LED = 0;
    BLUE_LED = 0;
    RED_LED = 0;
    FOUR_PIN = 0;

//    FIVE_EN_PIN = 1;
//    SIX_EN_PIN = 1;

    putsUart0("Mechatronics Lab 2\n");
    shell();
}
