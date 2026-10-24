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
#include "tm4c123gh6pm.h"
#include "initInterrupts.h"
#include "initMotors.h"
#include "main.h"
#include "wait.h"
#include "initTimers.h"

int32_t speed = 0 ;
int32_t dist = -1;
int32_t traveled = 0;
int8_t speedmax=100;
int16_t precision = 10000;
int32_t dist_left = 141526 ;
int32_t dist_right = 0 ;
int16_t pi = 3145;          //3.145 *10
int16_t angle =0;

uint8_t bit =0; int8_t code = 0; int8_t state=0;
uint32_t key=0;
uint8_t addy =0;
uint8_t addynot =0;
uint8_t data =0;
uint8_t datanot =0;
int32_t tau = 22500; //0.5625ms*(40000clk/ms)
int32_t time[35] = {0};
//int32_t time =0;
int8_t index = 0;
uint8_t phase = 0;
int32_t DistFromWall = -1;

bool flag = 0;
uint8_t Uint8addynotnot = 0;
uint8_t Uint8datanotnot = 0;

#define ms4p5 180000
#define ms9   360000
#define ms13  520000
#define ms14  560000

void initInterrupts(void)
{

    //SYSCTL_RCGCGPIO_R =  SYSCTL_RCGCGPIO_R3 | SYSCTL_RCGCGPIO_R5; //PORT D,F
    //_delay_cycles(3);

    //PORTA
    GPIO_PORTA_DEN_R |= PIR_OUT_MASK;              //enable pin
    GPIO_PORTA_DIR_R &= ~PIR_OUT_MASK;             //bit 3 are inputs
    //GPIO_PORTA_PUR_R |= PIR_OUT_MASK;              // enable internal pull-up

    GPIO_PORTA_IS_R  &= ~PIR_OUT_MASK;   //edges
    GPIO_PORTA_IBE_R &= ~PIR_OUT_MASK;   //single edge
//    GPIO_PORTA_IEV_R &= ~PIR_OUT_MASK;   //falling edge
    GPIO_PORTA_IEV_R |= PIR_OUT_MASK;    //rising edge
    GPIO_PORTA_IM_R  |= PIR_OUT_MASK;    //allows for same port interrupts
    GPIO_PORTA_ICR_R |= PIR_OUT_MASK;    //clears interrupts to call again
    NVIC_EN0_R = 1 << (INT_GPIOA-16);   // turn-on interrupt 19 (GPIOA)

    //PORT C
    GPIO_PORTC_DEN_R |= ECHO_MASK;              //enable pin
    GPIO_PORTC_DIR_R &= ~ECHO_MASK;             //bit 3 are inputs
    GPIO_PORTC_PDR_R |= ECHO_MASK;              // enable internal pull-down for SW1

    GPIO_PORTC_IS_R  &= ~ECHO_MASK;   //edges
    GPIO_PORTC_IBE_R &= ~ECHO_MASK;   //single edge
    //GPIO_PORTC_IEV_R &= ~ECHO_MASK;   //falling edge
    GPIO_PORTC_IM_R  |= ECHO_MASK;    //allows for same port interrupts
    GPIO_PORTC_ICR_R |= ECHO_MASK;    //clears interrupts to call again
    NVIC_EN0_R = 1 << (INT_GPIOC-16);   // turn-on interrupt 19 (GPIOD)

    //PORT D
    GPIO_PORTD_DIR_R &= ~IR_OUT_MASK;             //bit 3 are inputs
    GPIO_PORTD_DEN_R |= IR_OUT_MASK;              //enable pin
    GPIO_PORTD_PUR_R |= IR_OUT_MASK;              // enable internal pull-down for SW1

    GPIO_PORTD_IS_R  &= ~IR_OUT_MASK;   //edges
    GPIO_PORTD_IBE_R &= ~IR_OUT_MASK;   //single edge
    GPIO_PORTD_IEV_R &= ~IR_OUT_MASK;   //falling edge
    GPIO_PORTD_IM_R  |= IR_OUT_MASK;    //allows for same port interrupts
    GPIO_PORTD_ICR_R |= IR_OUT_MASK;    //clears interrupts to call again
    NVIC_EN0_R = 1 << (INT_GPIOD-16);   // turn-on interrupt 19 (GPIOD)

    //PORT F
    GPIO_PORTF_DIR_R &= ~PT_OUT1_MASK | ~PT_OUT2_MASK;            //bit 3 and 4 are inputs
    GPIO_PORTF_DEN_R |= PT_OUT1_MASK | PT_OUT2_MASK;              //enable pin
    GPIO_PORTF_PDR_R |= PT_OUT1_MASK | PT_OUT2_MASK;              // enable internal pull-down for SW1

    // Configure falling edge interrupts on row inputs
    GPIO_PORTF_IS_R  &= ~PT_OUT1_MASK | ~PT_OUT2_MASK; //edges
    GPIO_PORTF_IBE_R &= ~PT_OUT1_MASK | ~PT_OUT2_MASK; //single edge
    GPIO_PORTF_IEV_R &= ~PT_OUT1_MASK | ~PT_OUT2_MASK; //falling edge
    GPIO_PORTF_IM_R  |= PT_OUT1_MASK | PT_OUT2_MASK;   //allows for same port interrupts
    GPIO_PORTF_ICR_R |= PT_OUT1_MASK | PT_OUT2_MASK;   //clears interrupts to call again
    NVIC_EN0_R = 1 << (INT_GPIOF-16);                  // turn-on interrupt 46 (GPIOF)
}



void PortAISR(void)
{
    RED_LED ^= 1;
    waitMicrosecond(400000);
    RED_LED ^= 1;
    GPIO_PORTA_ICR_R |= PIR_OUT_MASK;
}

void PortFISR(void)
{

    //NVIC_EN0_R = 0 << (INT_GPIOF-16);     //turn off gpio int

    if (GPIO_PORTF_MIS_R & PT_OUT1_MASK)    //checks value of 2rd bit of MIS
    {
        //putsUart0("L \n");
        NVIC_EN0_R = 0 << (INT_GPIOF-16);   //turn off gpio int
        TIMER1_CTL_R |= TIMER_CTL_TAEN;     //oneshotISR1
        TIMER3_ICR_R = TIMER_ICR_TATOCINT;      //clears timer3ISR
        TIMER3_IMR_R = TIMER_IMR_TATOIM;
        dist_left += 45*pi;                 //4.5*10 * pi (each hole distance)
        if ((dist <= dist_left) &&(dist > 1))
        {
            move(0, 0, 0, 0);
        }
        traveled += 45*pi;
        //process_dist(dist_left,dist_right,speed,dist);
         //calls timer3ISR
        GPIO_PORTF_ICR_R  |= PT_OUT1_MASK; //clear interrupt to use again
        TIMER3_ICR_R = TIMER_ICR_TATOCINT;      //clears timer3ISR
    }

    if (GPIO_PORTF_MIS_R & PT_OUT2_MASK)    //checks value of 4th bit of MIS
    {
        //GREEN_LED ^= 1;
        //putsUart0("R \n");
        NVIC_EN0_R = 0 << (INT_GPIOF-16);   //turn off gpio int
        TIMER2_CTL_R |= TIMER_CTL_TAEN; //calls oneshotISR2

        GPIO_PORTF_ICR_R  |= PT_OUT2_MASK;
    }

}

void oneshotISR1(void) //T1A
{
    GPIO_PORTF_ICR_R  |= PT_OUT1_MASK;
    NVIC_EN0_R = 1 << (INT_GPIOF-16);       //turn on gpio int
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;      //clears timer interrupts to call again
    TIMER1_TAILR_R = 1000000;               // set load value to 1e5 for 40 Hz interrupt rate, 25ms
}
void oneshotISR2(void) //T2A
{

    GPIO_PORTF_ICR_R  |= PT_OUT2_MASK;
    NVIC_EN0_R = 1 << (INT_GPIOF-16);       //turn on gpio int
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;      //clears timer interrupts to call again
    TIMER2_TAILR_R = 1000000;               // set load value to 1e5 for 40 Hz interrupt rate, 25ms
}

void PortDISR()
{
//    if (GPIO_PORTF_MIS_R & IR_OUT_MASK)
//    {
        //NVIC_EN0_R = 0 << (INT_GPIOD-16);         //turn off gpio int
        time[index] = WTIMER3_TBV_R;
        index++;
        if (index > 35)
        {
            index = 0;
            dist_left=0;
            dist =0;
        }
        if (state == 0)  //idle state, zero counter and moves to next state
        {
            WTIMER3_TBV_R = 0;
            reset();
            key = 0;
            state++;
        }
        else if (state == 1) //next falling edge
        {
            if( (WTIMER3_TBV_R < ms13 ) || (WTIMER3_TBV_R > ms14) ) // read counter input, if outside of 13.5 range then invalid
            {
                state = 0;
            }
            else
            {
                state++;
                bit = 0;  //reset bit to do it again if from idle
                WTIMER3_TBV_R = 0;
            }
        }
        else if (state > 1)
        {
            if (bit <= 32)
            {
                if ( (WTIMER3_TBV_R > (1.5*tau)) && (WTIMER3_TBV_R < (2.5*tau)) )
                {
                    code = 0;
                }
                if ( (WTIMER3_TBV_R > 3.5*tau) && (WTIMER3_TBV_R < (4.5*tau)) )
                {
                    code = 1;
                }
                if (bit < 7)
                {
                    addy = (addy | code) << 1;
                }
                else if (bit == 7)
                {
                    addy = (addy | code);
                }
                else if (bit > 7  && bit < 15)
                {
                    addynot = (addynot | code) << 1;
                }
                else if (bit == 15)
                {
                    addynot = (addynot | code);
                }
                else if (bit > 15 && bit < 23)
                {
                    data = (data | code) << 1;
                }
                else if (bit == 23)
                {
                    data = (data | code);
                }
                else if (bit > 23 && bit < 31)
                {
                    datanot = (datanot | code) << 1;
                }
                if (bit < 31)
                {
                    key = (key | code) << 1;        //load code into key and then bit shift
                }
                else if (bit == 32)
                {
                    datanot = (datanot | code);
                    key = (key | code);
                }
                WTIMER3_TBV_R = 0;              //reset timer to go again
                bit++;
            }
            else
            {
                Uint8addynotnot = ~(addynot); //~(addynot) becomes signed for some reason, change back to uint 223
                Uint8datanotnot = ~(datanot);
                if( (bit >= 32) && (addy == Uint8addynotnot) && (data == Uint8datanotnot ) ) //bit becomes 33 due to last signal
                {
                    flag = 1;
                    state=0;
                }
                else
                {
                    flag = 0;
                    state=0;
                }
            }

        }
        //NVIC_EN0_R = 1 << (INT_GPIOD-16);
        GPIO_PORTD_ICR_R  |= IR_OUT_MASK;
//    }
//

}

void reset(void) //reset for next command
{
    state = 0;
    bit = 0;
    addy =0;
    addynot =0;
    data =0;
    datanot =0;
    Uint8addynotnot = 0;
    Uint8datanotnot = 0;
}

void resetKey(void)
{
    key = 0;
    flag = 0;
}

void distSet(int moveAmount)
{
    dist = moveAmount;
}

void reset_distance(void)
{
    dist = -1;
    dist_left = 300000; //141526
}
