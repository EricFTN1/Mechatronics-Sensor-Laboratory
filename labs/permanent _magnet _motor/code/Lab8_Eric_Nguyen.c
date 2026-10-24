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
#include <stdlib.h>
#include <stdbool.h>
//#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "initMotors.h"
#include "initTimers.h"
#include "initInterrupts.h"
#include "main.h"

//Interface Defines
#define MAX_CHARS 80
#define MAX_FIELDS 5
//#define DEBUG2 1
//#define DEBUG3 1

uint8_t c =0;
uint32_t nav[4] = {0};
bool NavOn = 0;

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
}   USER_DATA;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R0 |SYSCTL_RCGCGPIO_R1| SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R3 | SYSCTL_RCGCGPIO_R4 | SYSCTL_RCGCGPIO_R5; //PORT A,C,D,E,F
    _delay_cycles(3);

    // Configure LED+SLEEP pins (PORTF)
    GPIO_PORTF_DIR_R  |= RED_LED_MASK | GREEN_LED_MASK;                     // bit 1 output
    GPIO_PORTF_DR2R_R |= RED_LED_MASK | GREEN_LED_MASK;                     // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R  |= RED_LED_MASK | SLEEP_MASK;  // enable

    // Configure Sleep  PE3 (PORTE)
    GPIO_PORTE_DIR_R |= SLEEP_MASK;  // Output
    GPIO_PORTE_DEN_R |= SLEEP_MASK;  // enable SLEEP

    // Configure TRIG PE2 (PORTE)
    GPIO_PORTE_DIR_R |= TRIG_MASK;  // Output
    GPIO_PORTE_DEN_R |= TRIG_MASK;  // enable SLEEP
}

void getsUart0(USER_DATA* data)
{
    uint8_t count=0;
    while (true)
    {
        char character = getcUart0();   // get character from fifo, masking off the flags - FROM uart0.c/getcUart0()
        if ( (character == 8)||(character==127) && (count > 0))
        {
            count--; //decrement count so last character is overwritten
        }
        else if (character == 13) //if theres an enter
        {
            data->buffer[count]= '\0'; //add null terminator to end of string
            return;
        }
        else if (character >= 32) //32=space, printable character'>'32
        {
            data->buffer[count] = character;
            count++;
        }
        if (count == MAX_CHARS) //there is more characters than allowed
        {
            data->buffer[MAX_CHARS]= '\0'; //add null terminator to end of string
            return;
        }
    }
}

void parseFields(USER_DATA* data)
{
    uint8_t i=0;
    data->fieldCount=0;
    for (i=0; i<=MAX_CHARS; i++) //Implements delimiters as nulls
    {
        if (!(((data->buffer[i]>='A')&&(data->buffer[i]<='Z'))||((data->buffer[i]>='a')&&(data->buffer[i]<='z'))) ) //NOT Alpha
        {
            if( !((data->buffer[i]>=48)&&(data->buffer[i]<=57)) ) // NOT Numeric
            {
                data->buffer[i]= '\0';
            }
        }
    }

    i=0;
    for (i=0; i<=MAX_CHARS; i++) //Implements field type and position
    {
        if ( (((data->buffer[i]>='A')&&(data->buffer[i]<='Z')) || ((data->buffer[i]>='a')&&(data->buffer[i]<='z')))  ) //Alpha && previous is delim
        {
            if( (data->buffer[i-1]=='\0') )
            {
                data->fieldType[data->fieldCount]='A';
                data->fieldPosition[data->fieldCount]=i;
                data->fieldCount++;
            }
        }
        else if( ((data->buffer[i]>=48)&&(data->buffer[i]<=57)) ) //Numeric && previous is delim
        {
            if( (data->buffer[i-1]=='\0') )
            {
                data->fieldType[data->fieldCount]='N';
                data->fieldPosition[data->fieldCount]=i;
                data->fieldCount++;
            }
        }
    }

}
bool stringcompare(const char strCommand[],const char ValidCommand[])
{
    uint8_t i = 0;
    uint8_t strCommandCount =0;
    uint8_t ValidCommandCount =0;
    while ( (strCommand[i] != '\0') && (ValidCommand[i] != '\0') )
        {
            if( (strCommand[i]==ValidCommand[i]) ) //increments both if same
            {
                strCommandCount = strCommandCount + 1;
                ValidCommandCount = ValidCommandCount + 1;
            }
            i = i+1;
        }
    if ( (strCommandCount==i)&&(ValidCommandCount==i) ) //compare count with i
    {
        return true;
    }
    else
    {
        return false;
    }
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    if( ((fieldNumber < (MAX_FIELDS+1)) && (data->fieldType[fieldNumber]=='A')))
    {
        return &data->buffer[data->fieldPosition[fieldNumber]];
    }
    else
    {
        return NULL;
    }
}
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    if (data->fieldCount == (minArguments))
    {
        char* check = getFieldString(data,0);

        if (stringcompare(strCommand,check))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    if ( (fieldNumber <= MAX_FIELDS)&&(data->fieldType[fieldNumber]=='N') )
        {
        int32_t value = atoi(&data->buffer[data->fieldPosition[fieldNumber]]);
        return value;
        }
    return -1;
}

void Crank90s(void) //turn 90 degrees
{
    speed = speedmax;
    reset_distance();
    angle = 90;
    distSet((2*pi*675*angle)/360 - 30000);
    //dist = (2*pi*675*angle)/360 - 30000;
    move(0, speed, speed, 0);
}

uint32_t measure_mm(uint8_t n)
{
    int measure=0;
    return measure;
}

void toggleNavOn(void)
{
    NavOn ^= 1;
}

void navigation(NavOn)
{
    if (NavOn == 1)
    {
        Crank90s();
    }
    else
    {
        move(100, 100, 100, 100);
    }
}




//-----------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)

{

    initHw();
    initUart0();
    initMotors();
    initTimers();
    initInterrupts();
    setUart0BaudRate(115200,40e6);
    SLEEP_AL = 1; //sleep default off
    while (true)
    {
        if (kbhitUart0())
        {
            USER_DATA data = {0};
            getsUart0(&data);
            putcUart0('\n');
            uint8_t i;
            parseFields(&data);

            for (i = 0; i < data.fieldCount; i++)
            {
                putcUart0(data.fieldType[i]);
                putcUart0('\t');
                putsUart0(&data.buffer[data.fieldPosition[i]]);
                putcUart0('\n');
            }

            // Command evaluation

            if (isCommand(&data, "alert", 2)) //alert command
            {
                char* str = getFieldString(&data, 1);
                if (stringcompare(str,"on"))
                {
                    RED_LED = 1;
                }
                else if (stringcompare(str,"off"))
                {
                    RED_LED = 0;
                }
            }
            else if (isCommand(&data, "set", 3)) //set command
            {
                int32_t add = getFieldInteger(&data, 1);
                int32_t data2 = getFieldInteger(&data, 2);
                char* str1 = getFieldString(&data, 1);
                char* str2 = getFieldString(&data, 2);
                if (stringcompare(str1,"sleep"))
                {
                    if (stringcompare(str2,"on"))
                    {
                        SLEEP_AL = 0;
                    }
                    else if (stringcompare(str2,"off"))
                    {
                        SLEEP_AL = 1;
                    }
                }
            }
            else if (isCommand(&data, "forward", 1)) //set command
            {
                speed = speedmax;
                reset_distance(); //reset dist
                move(speed, 0, speed, 0);
            }

            else if (isCommand(&data, "forward", 2)) //set command
            {
                speed = getFieldInteger(&data, 1);
                reset_distance(); //reset dist
                move(speed, 0, speed, 0);
            }
            else if (isCommand(&data, "forward", 3)) //set command
            {
                speed = getFieldInteger(&data, 1);
                reset_distance();
                distSet((getFieldInteger(&data, 2))*precision);
                //dist = (getFieldInteger(&data, 2))*precision;
                move(speed, 0, speed, 0);
            }

            else if (isCommand(&data, "stop", 1)) //set command
            {
                move(0, 0, 0, 0);
                reset_distance();
            }
            else if (isCommand(&data, "reverse", 1)) //set command
            {
                speed = speedmax;
                reset_distance(); //reset dist
                move(0, speed, 0, speed);
            }

            else if (isCommand(&data, "reverse", 2)) //set command
            {
                speed = getFieldInteger(&data, 1);
                reset_distance(); //reset dist
                move(0, speed, 0, speed);
            }
            else if (isCommand(&data, "reverse", 3)) //set command
            {
                speed = getFieldInteger(&data, 1);
                reset_distance();
                distSet((getFieldInteger(&data, 2))*precision);
                //dist = (getFieldInteger(&data, 2))*precision;
                move(0, speed, 0, speed);
            }
            else if (isCommand(&data, "ccw", 1)) //set command
            {
                speed = speedmax;
                reset_distance();
                move(0, speed, speed, 0);
            }
            else if (isCommand(&data, "ccw", 2)) //set command
            {
                speed = speedmax;
                reset_distance();
                angle = getFieldInteger(&data, 1);
                if (angle <= 90)
                {
                    distSet((2*pi*675*angle)/360 - 30000);
                    //dist = (2*pi*675*angle)/360 - 30000;
                }
                else
                {
                    distSet((2*pi*675*angle)/360);
                    //dist = (2*pi*675*angle)/360;
                }
                move(0, speed, speed, 0);
            }
            else if (isCommand(&data, "cw", 1)) //set command
            {
                speed = speedmax;
                reset_distance();
                move(speed, 0, 0, speed);
            }
            else if (isCommand(&data, "cw", 2)) //set command
            {
                speed = speedmax;
                reset_distance();
                angle = getFieldInteger(&data, 1);
                if (angle <= 90)
                {
                    distSet((2*pi*675*angle)/360 - 30000);
                    //dist = (2*pi*675*angle)/360 - 30000;
                }
                else
                {
                    distSet((2*pi*675*angle)/360);
                    //dist = (2*pi*675*angle)/360;
                }
                move(speed, 0, 0, speed);
            }
        }
        if (flag)
        {
            if (key == 0x20DF6A95) //Netflix Button, navigation wip
            {
                move(0, 0, 0, 0); //stop
                toggleNavOn(); //toggle navigation
                navigation(NavOn);
                resetKey();
            }
        }

        if (flag && (NavOn == 0))
            {
                if (key == 0x20DF02FD)  //Forward button
                {
                    move(100, 0, 100, 0);
                }
                if (key == 0x20DF609F) //Right button cw
                {
                    move(100, 0, 0, 100);
                }
                if (key == 0x20DFE01F) //Left button ccw
                {
                    move(0, 100, 100, 0);
                }
                if (key == 0x20DF827D) //Reverse Button
                {
                    move(0, 100, 0, 100);
                }
                if (key == 0x20DF22DD) //Ok Button, stops
                {
                    move(100, 100, 100, 100);
                }
                if (key == 0x20DF6A95) //Netflix Button, navigation wip
                {
                    move(100, 100, 100, 100); //stop
                    toggleNavOn();
                    navigation(NavOn);
                      //toggle navigation
                }
                resetKey();
            }
    }
}

