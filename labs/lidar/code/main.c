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
#include "wait.h"
#include "uart0.h"
#include "uart1.h"
#include "gpio.h"
#include "tm4c123gh6pm.h"
#include "math.h"

// void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))


typedef struct _packet
{
    uint8_t payloadData[5];
} packet; // Dis some cool shit but I'm drunk

typedef struct _distAngle
{
    float angle;
    float dist;
}distAngle;


int compare(const void * processedData0, const void * processedData1){
    if(((distAngle *)processedData0)->angle < ((distAngle *)(processedData1))->angle){
        return -1;
    }
    if(((distAngle *)processedData0)->angle == ((distAngle *)(processedData1))->angle){
        return 0;
    }
    if(((distAngle *)processedData0)->angle > ((distAngle *)(processedData1))->angle){
        return +1;
    }
    return 0;
}

float slices[360];

#define START 0xA5
#define RX 0x5A
#define SCAN 0x20
#define HEIGHT 1000
#define STOP 0x25
#define INFO 0x50
#define PACKET_NUMBER 360

void initHw()
{
    initSystemClockTo40Mhz();
    _delay_cycles(3);

    initUart0();
    initUart1();
}

// Hi Eric!

void main(){
    packet rawData[360];
    uint8_t frame = 0;
    uint16_t angle = 0;
    uint16_t distance = 0;
    float ANGLE = 0;
    float DISTANCE = 0;
    uint32_t area = 0;
    distAngle processedData[360];
    float lastAngle;
    float lastDistance;
    char str[40];
    initHw();
    while(1){
//        putcUart1(0);
//        putcUart1(START);
//        putcUart1(INFO);
//        putcUart0('\n');
//        putcUart0('\n');
//
//        while(getcUart1() != 0xA5){};
//        puthUart0(0xA5);
//        putcUart0('\t');
        int i;
//        for(i = 0; i < 6; i++){
//            uint8_t info = getcUart1();
//            puthUart0(info);
//            putcUart0('\t');
//        }
//
//        putcUart0('\n');
//        putcUart0('\n');
//
//        for(i = 0; i < 19; i++){
//            uint8_t info = getcUart1();
//            puthUart0(info);
//            putcUart0('\t');
//        }

        putcUart1(0); // Flush the buffer
        putcUart1(START);
        putcUart1(SCAN); // Send the command
        while(getcUart1() != 0xA5){};
        //puthUart0(0xA5);
        for(i = 0; i < 6; i++){ // Grab the request descriptor
            uint8_t info = getcUart1();
            //puthUart0(info);
        }

        int j;
        for(j = 0; j < PACKET_NUMBER; j++){
            for(i = 0; i < 5; i++){ // Grab the data
                rawData[j].payloadData[i] = getcUart1();
            }
        }
        putcUart0('\n');
        putcUart0('\n');
        putsUart0("Slices: ");
        putcUart0('\n');
        area = 0;
        for(i = 0; i < PACKET_NUMBER; i++){
            frame = rawData[i].payloadData[1];
            angle = frame;
            angle >>= 1;
            frame = rawData[i].payloadData[2];
            angle |= frame << 7; // Fill the angle value, may be wrong endianness
            ANGLE = angle/64.0; // Unit conversion according to docs?



            frame = rawData[i].payloadData[3];
            distance = frame;
            frame = rawData[i].payloadData[4];
            distance |= frame << 8; // Fill the distance value, may be wrong endainness
            DISTANCE = distance/4.0; // Unit conversion according to docs?

            DISTANCE = DISTANCE/25.4;

            // 1/2 b * c * sin(a)

//            float s1 = sin(ANGLE-lastAngle);
//            float s2 = lastDistance*s1;
//            float s3 = DISTANCE*s2;
//            float s4 = 0.5*s3;
//
//
//            slices[i] = s4;
//            putcUart0('\n');
//            putiUart0((uint32_t)floor(slices[i]));
//            putcUart0('\n');
//            // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
//            area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r
//
//            lastAngle = ANGLE;
//            lastDistance = DISTANCE;

            processedData[i].angle = ANGLE;
            processedData[i].dist = DISTANCE;
        }

        qsort(&processedData, 360, sizeof(distAngle), &compare);

        // 1/2 b * c * sin(a)

        putcUart0('\n');
        putsUart0("ANGLE\tDISTANCE\n");
        snprintf(str, sizeof(str), "%f\t%f\n",processedData[0].angle,processedData[0].dist);
        putsUart0(str);

        float s1 = sin(processedData[0].angle);
        float s3 = processedData[0].dist*s1;
        float s4 = 0.5*s3;


        slices[0] = s4;
//        putcUart0('\n');
//        putiUart0((uint32_t)floor(slices[i]));
//        putcUart0('\n');
        // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)

        area = 0;
        area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r

        lastAngle = ANGLE;
        lastDistance = DISTANCE;

        for(i = 1; i < PACKET_NUMBER; i++){
            // 1/2 b * c * sin(a)

            snprintf(str, sizeof(str), "%f,%f\n",processedData[i].angle,processedData[i].dist);
            putsUart0(str);

            float s1 = sin((processedData[i].angle - processedData[i-1].angle)*M_PI/180);
            float s2 = processedData[i-1].dist*s1;
            float s3 = processedData[i].dist*s2;
            float s4 = 0.5*s3;


            slices[i] = s4;
//            putcUart0('\n');
//            putiUart0((uint32_t)floor(slices[i]));
//            putcUart0('\n');
            // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
            area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r

            lastAngle = ANGLE;
            lastDistance = DISTANCE;
        }

        putcUart0('\n');
        putcUart0('\n');
        putsUart0("Calculated Area: ");
        putiUart0(area);
        putcUart0('\n');







//        while(flag){
//            // All of the below is from page 16 of the datasheet (command guide) one of the 2 PDFs sent in the zip file called lidar that Losh sent. (The other pdf is kinda useless tbh, its just bookkeeping bs)
////            if(startBit[0] == 0xA5){ // Check for universal start bits
////                if(startBit[1] == 0x5A){ // Check for RX
//
//                    frame = getcUart1();
//                    angle = frame << 1;
//                    frame = getcUart1();
//                    angle |= frame << 8; // Fill the angle value, may be wrong endianness
//                    ANGLE = angle/64.0; // Unit conversion according to docs?
//
//                    frame = getcUart1();
//                    putiUart0(frame);
//                    lastDistance = DISTANCE;
//                    distance = frame;
//                    frame = getcUart1();
//                    distance |= frame <<8; // Fill the distance value, may be wrong endainness
//                    DISTANCE = distance/4.0; // Unit conversion according to docs?
//
//                    startBit[0] = getcUart1();
//                    startBit[1] = getcUart1();
//                    flag = getcUart1() & 0x01;
//                    // Start further processing
//
//                    // 1/2 b * c * sin(a)
//
//                    // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
//                    area += ((1/2)*DISTANCE*lastDistance*sin(ANGLE-lastAngle)); // The units should be correct according to the datasheet. Math is from google (but seems right from the part of my brain that thinks it remembers basic trig
////                }
////            }
//        }
//        area = 0; // Need to make sure this happens.
//        angle = 0;
//        lastDistance = 0;
//        distance = 0;
//        frame = 0; // The rest is just book keeping / double checking to make me feel better even though none of them really need to be zeroed based on execution order.
////        snprintf(str, sizeof(str), "Area: %d\n", area);
////        putsUart0(str);
////        snprintf(str, sizeof(str), "Height: %d\n", area*HEIGHT);
////        putsUart0(str);
//
//        putiUart0(area);

        putcUart1(START);
        putcUart1(STOP);
        waitMicrosecond(1000000);
    }

}























//
//
//
///*
// * main.c
// *
// *  Created on: Nov 8, 2024
// *      Author: clara
// */
//
////-------------------------------------------------------------------
//
////sbit ADDO = P1^5;
////sbit ADSK = P0^0;
////
////unsigned long ReadCount(void){
////    unsigned long Count;
////    unsigned char i;
////    ADDO=1;
////    ADSK=0;
////    Count=0;
////    while(ADDO);
////    for (i=0;i<24;i++){
////        ADSK=1;
////        Count=Count<<1;
////        ADSK=0;
////        if(ADDO) Count++;
////    }
////    ADSK=1;
////    Count=Count^0x800000;
////    ADSK=0;
////    return(Count);
////}
//#include <stdio.h>
//#include <stdint.h>
//#include "clock.h"
//#include "wait.h"
//#include "uart0.h"
//#include "uart1.h"
//#include "gpio.h"
//#include "tm4c123gh6pm.h"
//#include "math.h"
//
//// void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))
//
//qsort();
//
//typedef struct _packet
//{
//    uint8_t payloadData[5];
//} packet; // Dis some cool shit but I'm drunk
//
//typedef struct _distAngle
//{
//    float angle;
//    float dist;
//}distAngle;
//
//
//int compare(const void * processedData0, const void * processedData1){
//    if(((distAngle *)processedData0)->angle > ((distAngle *)(processedData1))->angle){
//        return -1;
//    }
//    if(((distAngle *)processedData0)->angle == ((distAngle *)(processedData1))->angle){
//        return 0;
//    }
//    if(((distAngle *)processedData0)->angle < ((distAngle *)(processedData1))->angle){
//        return +1;
//    }
//    return 0;
//}
//
//float slices[360];
//
//#define START 0xA5
//#define RX 0x5A
//#define SCAN 0x20
//#define HEIGHT 1000
//#define STOP 0x25
//#define INFO 0x50
//#define PACKET_NUMBER 360
//
//void initHw()
//{
//    initSystemClockTo40Mhz();
//    _delay_cycles(3);
//
//    initUart0();
//    initUart1();
//}
//
//// Hi Eric!
//
//void main(){
//    packet rawData[360];
//    uint8_t frame = 0;
//    uint16_t angle = 0;
//    uint16_t distance = 0;
//    float ANGLE = 0;
//    float lastAngle = 0;
//    float DISTANCE = 0;
//    float lastDistance = 0;
//    uint32_t area = 0;
//    distAngle processedData[360];
//    initHw();
//    while(1){
//        putcUart1(0);
//        putcUart1(START);
//        putcUart1(INFO);
//        putcUart0('\n');
//        putcUart0('\n');
//
//        while(getcUart1() != 0xA5){};
//        puthUart0(0xA5);
//        putcUart0('\t');
//        int i;
//        for(i = 0; i < 6; i++){
//            uint8_t info = getcUart1();
//            puthUart0(info);
//            putcUart0('\t');
//        }
//
//        putcUart0('\n');
//        putcUart0('\n');
//
//        for(i = 0; i < 19; i++){
//            uint8_t info = getcUart1();
//            puthUart0(info);
//            putcUart0('\t');
//        }
//
//        putcUart1(0); // Flush the buffer
//        putcUart1(START);
//        putcUart1(SCAN); // Send the command
//        putcUart0('\n');
//        putcUart0('\n');
//
//        for(i = 0; i < 7; i++){ // Grab the request descriptor
//            uint8_t info = getcUart1();
//            puthUart0(info);
//            putcUart0('\t');
//        }
//
//        int j;
//        for(j = 0; j < PACKET_NUMBER; j++){
//            // Get a packet
//
//            putcUart0('\n');
//            putcUart0('\n');
//
//            for(i = 0; i < 5; i++){ // Grab the data
//                rawData[j].payloadData[i] = getcUart1();
//                puthUart0(rawData[j].payloadData[i]);
//                putcUart0('\t');
//            }
//        }
//        putcUart0('\n');
//        putcUart0('\n');
//        putsUart0("Slices: ");
//        putcUart0('\n');
//        for(i = 0; i < PACKET_NUMBER; i++){
//            frame = rawData[i].payloadData[1];
//            angle = frame;
//            angle >>= 1;
//            frame = rawData[i].payloadData[2];
//            angle |= frame << 6; // Fill the angle value, may be wrong endianness
//            ANGLE = angle/64.0; // Unit conversion according to docs?
//            if(ANGLE == 0){
//                ANGLE = 1;
//            }
//
//
//            frame = rawData[i].payloadData[3];
//            distance = frame;
//            frame = rawData[i].payloadData[4];
//            distance |= frame << 8; // Fill the distance value, may be wrong endainness
//            DISTANCE = distance/4.0; // Unit conversion according to docs?
//            if(DISTANCE == 0){
//                DISTANCE = 1;
//            }
//            DISTANCE /= 25.4;
//
//            // 1/2 b * c * sin(a)
//
////            float s1 = sin(ANGLE-lastAngle);
////            float s2 = lastDistance*s1;
////            float s3 = DISTANCE*s2;
////            float s4 = 0.5*s3;
////
////
////            slices[i] = s4;
////            putcUart0('\n');
////            putiUart0((uint32_t)floor(slices[i]));
////            putcUart0('\n');
////            // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
////            area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r
////
////            lastAngle = ANGLE;
////            lastDistance = DISTANCE;
//
//            processedData[i].angle = ANGLE;
//            processedData[i].dist = DISTANCE;
//        }
//
//        qsort(&processedData, 360, sizeof(distAngle), &compare);
//
//        // 1/2 b * c * sin(a)
//
//        float s1 = sin(processedData[PACKET_NUMBER-1].angle);
//        float s3 = processedData[PACKET_NUMBER-1].dist*s1;
//        float s4 = 0.5*s3;
//
//
//        slices[0] = s4;
//        putcUart0('\n');
//        putiUart0((uint32_t)floor(slices[i]));
//        putcUart0('\n');
//        // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
//        area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r
//
//        lastAngle = ANGLE;
//        lastDistance = DISTANCE;
//
//        for(i = PACKET_NUMBER - 2; i >0; i--){
//            // 1/2 b * c * sin(a)
//
//            float s1 = processedData[i].angle-processedData[i-1].angle;
//            float s2 = processedData[i-1].dist*s1;
//            float s3 = processedData[i].dist*s2;
//            float s4 = 0.5*s3;
//
//
//            slices[i] = s4;
//            putcUart0('\n');
//            putiUart0((uint32_t)floor(slices[i]));
//            putcUart0('\n');
//            // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
//            area += (uint32_t)floor(slices[i]);// The units should be correct according to the datasheet. Math is from google (but seems r
//
//            lastAngle = ANGLE;
//            lastDistance = DISTANCE;
//        }
//
//        putcUart0('\n');
//        putcUart0('\n');
//        putsUart0("Calculated Area: ");
//        putiUart0(area);
//        putcUart0('\n');
//
//
//
//
//
////        while(flag){
////            // All of the below is from page 16 of the datasheet (command guide) one of the 2 PDFs sent in the zip file called lidar that Losh sent. (The other pdf is kinda useless tbh, its just bookkeeping bs)
//////            if(startBit[0] == 0xA5){ // Check for universal start bits
//////                if(startBit[1] == 0x5A){ // Check for RX
////
////                    frame = getcUart1();
////                    angle = frame << 1;
////                    frame = getcUart1();
////                    angle |= frame << 8; // Fill the angle value, may be wrong endianness
////                    ANGLE = angle/64.0; // Unit conversion according to docs?
////
////                    frame = getcUart1();
////                    putiUart0(frame);
////                    lastDistance = DISTANCE;
////                    distance = frame;
////                    frame = getcUart1();
////                    distance |= frame <<8; // Fill the distance value, may be wrong endainness
////                    DISTANCE = distance/4.0; // Unit conversion according to docs?
////
////                    startBit[0] = getcUart1();
////                    startBit[1] = getcUart1();
////                    flag = getcUart1() & 0x01;
////                    // Start further processing
////
////                    // 1/2 b * c * sin(a)
////
////                    // This sums up all of the itty bitty areas :) THEY'RE SO SMALL AND CUTE (i'm drunk)
////                    area += ((1/2)*DISTANCE*lastDistance*sin(ANGLE-lastAngle)); // The units should be correct according to the datasheet. Math is from google (but seems right from the part of my brain that thinks it remembers basic trig
//////                }
//////            }
////        }
////        area = 0; // Need to make sure this happens.
////        angle = 0;
////        lastDistance = 0;
////        distance = 0;
////        frame = 0; // The rest is just book keeping / double checking to make me feel better even though none of them really need to be zeroed based on execution order.
//////        snprintf(str, sizeof(str), "Area: %d\n", area);
//////        putsUart0(str);
//////        snprintf(str, sizeof(str), "Height: %d\n", area*HEIGHT);
//////        putsUart0(str);
////
////        putiUart0(area);
//
//        putcUart1(START);
//        putcUart1(STOP);
//        waitMicrosecond(1000000);
//    }
//
//}





