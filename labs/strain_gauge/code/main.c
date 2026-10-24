/*
 * main.c
 *
 *  Created on: Nov 8, 2024
 *      Author: clara
 */

//-------------------------------------------------------------------

//sbit ADDO = P1^5;
//sbit ADSK = P0^0;
//
//unsigned long ReadCount(void){
//    unsigned long Count;
//    unsigned char i;
//    ADDO=1;
//    ADSK=0;
//    Count=0;
//    while(ADDO);
//    for (i=0;i<24;i++){
//        ADSK=1;
//        Count=Count<<1;
//        ADSK=0;
//        if(ADDO) Count++;
//    }
//    ADSK=1;
//    Count=Count^0x800000;
//    ADSK=0;
//    return(Count);
//}
#include <stdio.h>
#include <stdint.h>
#include "clock.h"
//#include "pain.h"
#include "wait.h"
#include "math.h"
#include "uart0.h"
#include "gpio.h"
#include "tm4c123gh6pm.h"

#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4))) //PF3


//#define EZERO    PORTE,0 // off-board red LED
#define EZERO    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 0*4))) //PE0
#define EZERO_MASK 1

uint32_t code = 0;
char str[80];

void initHw()
{
    initSystemClockTo40Mhz();
    _delay_cycles(3);
    enablePort(PORTF);
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    selectPinDigitalInput(PORTF, 3);
    GPIO_PORTE_DIR_R |= EZERO_MASK;  // Output
    GPIO_PORTE_DEN_R |= EZERO_MASK;  // enable

    //initPain();
    initUart0();
}
void main(){


    uint64_t avgArray[10] = {0};
    uint64_t sum = 0;
    uint64_t avg =0;
    uint32_t avgConv = 0;
    uint32_t grams =0;
    uint32_t newtons =0;
    int j = 0;
    initHw();
    putsUart0("LAB 7 MECHATRONICS\n");

    while(1){
        waitMicrosecond(250000);
        uint8_t i= 0;
        while(GREEN_LED);
        for(i = 0; i < 24; i++)     //24clock signals to get data
        {
            EZERO = 1;
            waitMicrosecond(20); // Square wave generation
            if(GREEN_LED == 0){ // Length of 1 is 90000 measured from registers
                code = code << 1;
                code |= 1;
            }
            else if(GREEN_LED == 1){ // Length of 0 is 45500 measured from registers
                code = code << 1;
            }
            EZERO = 0;
            waitMicrosecond(1);
        }

        EZERO = 1; //25the clock to indicate want to sample the A channel with 128 gain
        waitMicrosecond(20);
        EZERO = 0;
        waitMicrosecond(1);
//        putcUart0('\n');
//        snprintf(str, sizeof(str), "RAW: %d\n", code);
//        putsUart0(str);

        avgArray[j] = code;
        sum += avgArray[j]; //sums all values
        j++;
        if (j >= 10)
        {
            j = 0;
            avg = sum/10; //gets average value
            avgConv = avg; //64t to 32t
            sum =0;
            putcUart0('\n');
            snprintf(str, sizeof(str), "AVERAGE: %d\n", avgConv);
            putsUart0(str);

        }
        if ( (avgConv != 0) && ((abs((int32_t)(code - avgConv)))>10000) ) //checks for large change
        {
            grams = ceil((int32_t)(code - avgConv))/10000*225;
            newtons = grams * 101.971621; //https://www.convertunits.com/from/gram/to/newtons constant according to this website
        }

        if ( avgConv != 0 ) //checks for large change
        {
            snprintf(str, sizeof(str), "GRAMS: %d\n", grams);
            putsUart0(str);
            snprintf(str, sizeof(str), "FORCE (NEWTONS): %d\n", newtons);
            putsUart0(str);
        }

        code = 0;
    }
}
