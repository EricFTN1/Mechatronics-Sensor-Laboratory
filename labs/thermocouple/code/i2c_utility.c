// I2C Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz
// Stack:           4096 bytes (needed for snprintf)

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

// I2C devices on I2C bus 0 with 2kohm pullups on SDA (PB3) and SCL (PB2)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "uart0.h"
#include "i2c0.h"
#include "Lookup.h"
#include "wait.h"

// Range of polled devices
// 0 for general call, 1-3 for compatible i2c variants
// 120-123 are for 10-bit address mode
// 123-127 reserved
#define MIN_I2C_ADD 0x08
#define MAX_I2C_ADD 0x77

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

#define TEMP_CONST 1

#define MAX_CHARS 80
int main(void){
    char str[80];


    // Initialize hardware
    initHw();
    initUart0();
    initI2c0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    putsUart0("I2C0 Utility\n");


    uint8_t datacfgtem[2] = {0x8F,0x03};

    uint8_t datacfgtmp[2] = {0xE5,0x03};

    uint8_t datatmp[2];
    uint8_t datatem[2];


    while (true)
    {


        // 0xE503 TMP36 Read
        writeI2c0Registers(0x48, 0x01, datacfgtmp, 2);
        waitMicrosecond(500000);
        readI2c0Registers(0x48, 0x00, datatmp, 2);

        float vout2 = (((int16_t)((datatmp[0] << 8) + (datatmp[1]))*(float)62.5)/1000000);


        //0x8F03 Thermocouple Read

        writeI2c0Registers(0x48, 0x01, datacfgtem, 2);
        waitMicrosecond(500000);
        readI2c0Registers(0x48, 0x00, datatem, 2);

        float vout = ((int16_t)((datatem[0] << 8) + (datatem[1]))*(float)7.8125)/1000;

        // The 1000000 is for converting to

        //PART 4, TMP to get Tcj and then convery to Vcj
        snprintf(str, sizeof(str), "TMP36 Raw(Voltage): %f\n", vout2);
        putsUart0(str);
        putcUart0('\n');
        snprintf(str, sizeof(str), "TMP36 Converted to Temp: %f\n", ((vout2-0.5)/10e-3));
        putsUart0(str);
        putcUart0('\n');

        float Vcj = TempToAnalog((vout2-0.5)/10e-3);
        snprintf(str, sizeof(str), "TMP36 Temp converted to Vcj: %f\n", Vcj);
        putsUart0(str);
        putcUart0('\n');

        //Part 5 Thermalcouple Vtc + Vcj converted to temp
        snprintf(str, sizeof(str), "Thermocouple Raw: %f\n", vout);
        putsUart0(str);
        putcUart0('\n');
        snprintf(str, sizeof(str), "ThermoRaw + Vcj: %f\n", vout + Vcj);
        putsUart0(str);
        putcUart0('\n');
        int test5 = (AnalogToTemp(vout + Vcj));
        if (test5 == -1)
        {
            snprintf(str, sizeof(str), "Disconnected", (AnalogToTemp(vout + Vcj)));
                        putsUart0(str);
                        putcUart0('\n');
        }
        else
        {
            snprintf(str, sizeof(str), "Adjusted Temp: %.1f\n", (AnalogToTemp(vout + Vcj)));
            putsUart0(str);
            putcUart0('\n');
        }

    }
}

void getsUart0(char str[], uint8_t size)
{
    uint8_t count = 0;
    bool end = false;
    char c;
    while(!end)
    {
        c = getcUart0();
        end = (c == 13) || (count == size);
        if (!end)
        {
            if ((c == 8 || c == 127) && count > 0)
                count--;
            if (c >= ' ' && c < 127)
                str[count++] = c;
        }
    }
    str[count] = '\0';
}

uint8_t asciiToUint8(const char str[])
{
    uint8_t data;
    if (str[0] == '0' && tolower(str[1]) == 'x')
        sscanf(str, "%hhx", &data);
    else
        sscanf(str, "%hhu", &data);
    return data;
}

//        getsUart0(strInput, MAX_CHARS);
//        token = strtok(strInput, " \r\n");
//        ok = token != NULL;
//        valid = false;
//        regUsed = false;
//        if (strcmp(token, "write") == 0)
//        {
//            valid = true;
//            // Add
//            token = strtok(NULL, " ");
//            ok = ok && token != NULL;
//            add = asciiToUint8(token);
//            // Reg or Data
//            token = strtok(NULL, " ");
//            ok = ok && token != NULL;
//            arg = asciiToUint8(token);
//            // Optional Data
//            token = strtok(NULL, " \r\n");
//            // Determine if write add reg data or write add data
//            if (strlen(token) > 0)
//            {
//                reg = arg;
//                data = asciiToUint8(token);
//                regUsed = true;
//            }
//            else
//                data = arg;
//            if (ok)
//            {
//                if (regUsed)
//                {
//                    writeI2c0Register(add, reg, data);
//                    snprintf(str, sizeof(str), "Writing 0x%02"PRIX8" to address 0x%02"PRIX8", register 0x%02"PRIX8"\n", data, add, reg);
//                }
//                else
//                {
//                    writeI2c0Data(add, data);
//                    snprintf(str, sizeof(str), "Writing 0x%02"PRIX8" to address 0x%02"PRIX8"\n", data, add);
//                }
//                putsUart0(str);
//            }
//            else
//                putsUart0("Error in write command arguments\n");
//        }
//        if (strcmp(token, "read") == 0)
//        {
//            valid = true;
//            // Add
//            token = strtok(NULL, " ");
//            ok = ok && token != NULL;
//            add = asciiToUint8(token);
//            // Optional register
//            token = strtok(NULL, " \r\n");
//            // Determine if read add or read add reg
//            if (strlen(token) > 0)
//            {
//                reg = asciiToUint8(token);
//                regUsed = true;
//            }
//            if (ok)
//            {
//                if (regUsed)
//                {
//                    data = readI2c0Register(add, reg);
//                    snprintf(str, sizeof(str), "Read 0x%02hhX from address 0x%02"PRIX8", register 0x%02"PRIX8"\n", data, add, reg);
//                }
//                else
//                {
//                    data = readI2c0Data(add);
//                    snprintf(str, sizeof(str), "Read 0x%02"PRIX8" from address 0x%02"PRIX8"\n", data, add);
//                }
//                putsUart0(str);
//            }
//            else
//                putsUart0("Error in read command arguments\n");
//        }
//        if (strcmp(token, "poll") == 0)
//        {
//            valid = true;
//            found = false;
//            putsUart0("Devices found: ");
//            for (i = MIN_I2C_ADD; i <= MAX_I2C_ADD; i++)
//            {
//                if (pollI2c0Address(i))
//                {
//                    found = true;
//                    snprintf(str, sizeof(str), "0x%02"PRIX8" ", i);
//                    putsUart0(str);
//                }
//            }
//            if (!found)
//                putsUart0("(none)");
//            putsUart0("\n");
//        }
//        if (strcmp(token, "help") == 0)
//        {
//            valid = true;
//            putsUart0("Commands:\n");
//            putsUart0("    poll                  Poll devices on bus\n");
//
//            putsUart0("    read ADD REG          Read a byte from a device register\n");
//            putsUart0("    write ADD REG DATA    Write a byte to a device register\n");
//
//            putsUart0("    read ADD              Read a byte from a device\n");
//            putsUart0("    write ADD DATA        Write a byte to a device\n");
//        }
//        if (!valid)
//            putsUart0("Invalid command\n");
//    }
