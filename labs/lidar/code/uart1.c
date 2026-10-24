// UART1 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"

// PortA masks
#define UART_TX_MASK 32
#define UART_RX_MASK 16

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize UART1
void initUart1()
{
    // Enable clocks
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
    _delay_cycles(3);

    // Configure UART1 pins
    GPIO_PORTC_DEN_R |= UART_TX_MASK | UART_RX_MASK;    // enable digital on UART1 pins
    GPIO_PORTC_AFSEL_R |= UART_TX_MASK | UART_RX_MASK;  // use peripheral to drive PA0, PA1
    GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M); // clear bits 0-7
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC5_U1TX | GPIO_PCTL_PC4_U1RX;
                                                        // select UART1 to drive pins PA0 and PA1: default, added for clarity

    // Configure UART1 to 115200 baud (assuming fcyc = 40 MHz), 8N1 format
    UART1_CTL_R = 0;                                    // turn-off UART1 to allow safe programming
    UART1_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
    UART1_IBRD_R = 21;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART1_FBRD_R = 45;                                  // round(fract(r)*64)=45
    UART1_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART1_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                                                        // enable TX, RX, and module
}

// Set baud rate as function of instruction cycle frequency
void setUart1BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes128 = (fcyc * 8) / baudRate;   // calculate divisor (r) in units of 1/128,
                                                        // where r = fcyc / 16 * baudRate
    divisorTimes128 += 1;                               // add 1/128 to allow rounding
    UART1_CTL_R = 0;                                    // turn-off UART1 to allow safe programming
    UART1_IBRD_R = divisorTimes128 >> 7;                // set integer value to floor(r)
    UART1_FBRD_R = ((divisorTimes128) >> 1) & 63;       // set fractional value to round(fract(r)*64)
    UART1_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART1_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                                                        // turn-on UART1
}

// Blocking function that writes a serial character when the UART buffer is not full
void putcUart1(char c)
{
    while (UART1_FR_R & UART_FR_TXFF);               // wait if uart0 tx fifo full
    UART1_DR_R = c;                                  // write character to fifo
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart1(char* str)
{
    uint8_t i = 0;
    while (str[i] != '\0')
        putcUart1(str[i++]);
}

void puthUart1(uint32_t x)
{
//    char integer[12];
//    integer[11] = 0;
    putcUart1('0');
    putcUart1('x');
    char integer[9];
    integer[8] = 0;
    uint8_t i = 7;
    integer[i] = (x % 16) + 48;
    x /= 16;
    if(integer[i] > 57){
        switch(integer[i]){
            case 58:
                integer[i] = 'A';
                break;
            case 59:
                integer[i] = 'B';
                break;
            case 60:
                integer[i] = 'C';
                break;
            case 61:
                integer[i] = 'D';
                break;
            case 62:
                integer[i] = 'E';
                break;
            case 63:
                integer[i] = 'F';
                break;
            default:
                break;
        }
    }
    i--;
    while(x){
        integer[i] = (x % 16) + 48;
        x /= 16;
        if(integer[i] > 57){
            switch(integer[i]){
                case 58:
                    integer[i] = 'A';
                    break;
                case 59:
                    integer[i] = 'B';
                    break;
                case 60:
                    integer[i] = 'C';
                    break;
                case 61:
                    integer[i] = 'D';
                    break;
                case 62:
                    integer[i] = 'E';
                    break;
                case 63:
                    integer[i] = 'F';
                    break;
                default:
                    break;
            }
        }
        i--;
    }
    i++;
    while (integer[i] != '\0')
        putcUart1(integer[i++]);
}

void putiUart1(uint32_t x)
{
//    char integer[12];
//    integer[11] = 0;
    char integer[11];
    integer[10] = 0;
    uint8_t i = 9;
    integer[i] = (x % 10) + 48;
    x /= 10;
    i--;
    while(x){
        integer[i] = (x % 10) + 48;
        x /= 10;
        i--;
    }
    i++;
    while (integer[i] != '\0')
        putcUart1(integer[i++]);
}

// Blocking function that returns with serial data once the buffer is not empty
char getcUart1()
{
    while (UART1_FR_R & UART_FR_RXFE);               // wait if uart0 rx fifo empty
    return UART1_DR_R & 0xFF;                        // get character from fifo, masking off the flags
}

// Returns the status of the receive buffer
bool kbhitUart1()
{
    return !(UART1_FR_R & UART_FR_RXFE);
}
