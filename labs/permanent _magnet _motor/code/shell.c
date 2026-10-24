/*
 * shell.c
 *
 *  Created on: Sep 1, 2024
 *      Author: clara
 */

#include "commonui.h"
#include "systemph.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "pain.h"
#include "math.h"
#include <stdint.h>
#include <stdbool.h>
#include "wait.h"
#include "motorcontrol.h"


#define FOUR_PIN      (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4)))

double angle_accumulator = 0;
uint32_t STEP_TIME = 50000;
double maxDegrees = 55;
uint32_t STEP_SIZE = 1; //1 = full steps, 2 half, 4 quarter
double degrees = 0;
bool direction = true; //default ccw

uint32_t pwmCoilA = 0;
uint32_t pwmCoilB = 0;
char * procs[2] = {"hello", "world"};
uint8_t numprocs = 2;
float steps = 0 ;
double convsteps;

float fabsolute(float x){
    return sqrt(x*x);
}

void getStringUart(USER_DATA * data){
    int count = 0; // Make sure that count matches a valid index of the buffer
    char current_char = 0;
    while(current_char < 32){
        if(kbhitUart0()){
            current_char = getcUart0();
        }
        yield();
    }
    data->buffer[count] = current_char;
    count++;
    while(current_char != 13)
    {
        if(kbhitUart0()){// skip if uart0 rx fifo empty
            current_char = getcUart0();               // Fill char buffer with char from fifo w/ masked off flags
            if((current_char == 8) || (current_char == 127)){
                count--;
            }
            else{
                data->buffer[count] = current_char;
                count++;
                if(count == 80) break;
            }
        }
        yield();
    }
    data->buffer[count] = 0; //add null terminator to end of string
    return; // else return
}




void coilSwap(float sine, float cosine, float siner, float cosiner){

    if((cosiner == 1) && (siner == 0)){
        GREEN_LED = 1;
        BLUE_LED = 0;
        if(angle_accumulator > 64*M_PI){
            angle_accumulator = angle_accumulator - M_PI;
        }
    }
    else if((cosiner == 1) && (siner == 0)){
        RED_LED = 1; //+
        FOUR_PIN = 0; //-
    }
    else if((cosiner == 1) && (siner == 0)){
        GREEN_LED = 0; // +
        BLUE_LED = 1; // -
    }
    else if((cosiner == 1) && (siner == 0)){
        RED_LED = 0;
        FOUR_PIN = 1;
    }
    else if ( (cosine > 0) && (sine > 0) ){ // Coil B
        //  +-+-
        GREEN_LED = 1;
        BLUE_LED = 0;
        RED_LED = 1;
        FOUR_PIN = 0;
    }
    else if ( (cosine < 0) && (sine > 0) ){ // Coil B
        //  -++-
        GREEN_LED = 0; // -
        BLUE_LED = 1; // +
        RED_LED = 1; //+
        FOUR_PIN = 0; //-
    }
    else if ((cosine < 0) && (sine < 0) ){
          // -+-+
        GREEN_LED = 0; // +
        BLUE_LED = 1; // -
        RED_LED = 0;
        FOUR_PIN = 1;
    }
    else if ( (cosine > 0) && (sine < 0) ){
            // +--+
        GREEN_LED = 1;
        BLUE_LED = 0;
        RED_LED = 0;
        FOUR_PIN = 1;
    }
}

double degrees_to_steps(double correcteddegrees){
    convsteps = ceil(((double)fabsolute(correcteddegrees)/(1.8))*STEP_SIZE);
    return convsteps;
}

void microStep(double stepsneeded){
    while(stepsneeded > 0){
        waitMicrosecond(STEP_TIME/STEP_SIZE);

        float sine = sin(angle_accumulator);
        float cosine = cos(angle_accumulator);

        pwmCoilA = fabsolute(floor(1023*sine));
        pwmCoilB = fabsolute(floor(1023*cosine));
        motorSpeedFw(pwmCoilA);
        motorSpeedBw(pwmCoilB);

        coilSwap(sine, cosine, round(sine), round(cosine));

        if(direction){ //ccw
            angle_accumulator = angle_accumulator - (M_PI/(2*STEP_SIZE));
        }
        else{ //cw
            angle_accumulator = angle_accumulator + (M_PI/(2*STEP_SIZE));
        }



        stepsneeded--;
    }
    steps = 0 ;
}

void calibrate(){
        direction = false; //ccw
        while(!COLLECTOR){ // CW COIL ORDER - Goes CW until sensor is detected
            microStep(1);
        }
        direction = true; //ccw
        //angle_accumulator = angle_accumulator - (90/STEP_SIZE); //go back one mag field
        steps = floor(degrees_to_steps(45));
        microStep(steps);
        degrees = 0;
}

// 90deg is full step
// 45deg is half step
// 22.5 deg is quarter step

// For sine/cos step in degrees, its 90/STEP_SIZE

// 4*STEP_SIZE for total unit circle traversal, and STEP_SIZE for number of iterations between coil switches

// At the very start both coils should have polarity set

void shell(){
    calibrate();
    USER_DATA data;
    while(true){
        data.fieldCount = 0;
        getStringUart(&data);
        parseFields(&data);

        if(isCommand(&data, "ps", 1)){
            ps();
        }
        else if(isCommand(&data, "rotate", 2)){
            double newdegs = getFieldInteger(&data, 2);
            if(newdegs > 0){
                newdegs += .01;
            }
            else{
                newdegs -= .01;
            }
            if(newdegs < degrees){ //degress is current degrees
                if (direction == false){
                    angle_accumulator = angle_accumulator - (M_PI/(2*STEP_SIZE));
                }
                direction = true; //turning ccw now


            }
            else if(newdegs > degrees){

                if (direction == true){
                    angle_accumulator = angle_accumulator + (M_PI/(2*STEP_SIZE));
                }
                direction = false; //turning cw now
            }
            if(newdegs > maxDegrees){
                microStep(degrees_to_steps(maxDegrees - degrees));
                degrees = maxDegrees;
            }
            else if(newdegs < -1*maxDegrees){
                microStep(degrees_to_steps(-1*maxDegrees - degrees));
                degrees = -1*maxDegrees;
            }
            microStep(degrees_to_steps(newdegs - degrees));
            degrees = newdegs;
        }
        else if(isCommand(&data, "stepsize", 2)){

            STEP_SIZE = getFieldInteger(&data, 2);
        }
        else if(isCommand(&data, "waittime", 2)){
            STEP_TIME = getFieldInteger(&data, 2);
        }
        else if(isCommand(&data, "reboot", 1)){
            NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
        }
        else if(isCommand(&data, "ipcs", 1)){
            ipcs();
        }
        else if(isCommand(&data, "kill", 2)){
            uint32_t pid = getFieldInteger(&data, 2);
            kill(pid);
        }
        else if(isCommand(&data, "pi", 2)){
            char * command = getFieldString(&data, 2);
            if(!strcmp1(command,"ON")){
                pibool(1);
            }
            else{
                pibool(0);
            }
        }
        else if(isCommand(&data, "pkill", 2)){
            char * command = getFieldString(&data, 2);
            pkill(command);
        }
        else if(isCommand(&data, "preempt", 2)){
            char * command = getFieldString(&data, 2);
            if(!strcmp1(command,"ON")){
                preempt(1);
            }
            else{
                preempt(0);
            }
        }
        else if(isCommand(&data, "sched", 2)){
            char * command = getFieldString(&data, 2);
            if(!strcmp1(command,"PRIO")){
                sched(1);
            }
            else{
                sched(0);
            }
        }
        else if(isCommand(&data, "pidof", 2)){
            char * command = getFieldString(&data, 2);
            pidof(command);
        }
        else{
            char * command = getFieldString(&data, 1);
            uint8_t iter;
            for(iter = 0; iter < numprocs; iter++){
                if(!strcmp1(command,procs[iter])){
                    RED_LED = 1;
                    iter = numprocs;
                }
                else{
                    RED_LED = 0;
                }
            }
            if(!RED_LED){
                putsUart0("Invalid command\n");
            }
        }
    }
}




//
//if(newdegs < degrees){ //degress is current degrees
//    if (direction == false){
//        angle_accumulator = angle_accumulator - (M_PI/(2*STEP_SIZE));
//    }
//    direction = true; //turning ccw now
//
//
//}
//else if(newdegs > degrees){
//
//    if (direction == true){
//        angle_accumulator = angle_accumulator + (M_PI/(2*STEP_SIZE));
//    }
//    direction = false; //turning cw now
//}




//void coilswapCw(float current_angle){ //+90 for cw, -90 for ccw
//
//
//    if (current_angle == 0){ // Coil A
//        //  +-**
//        GREEN_LED = 1;
//        BLUE_LED = 0;
////        RED_LED = 0;
////        FOUR_PIN = 0;
//    }
//
//    else if (current_angle == 90){ // Coil B
//        //  **+-
////        GREEN_LED = 0;
////        BLUE_LED = 0;
//        RED_LED = 1; //+
//        FOUR_PIN = 0; //-
//    }
//
//    else if (current_angle == 180){
//        // -+**
//        GREEN_LED = 0; // +
//        BLUE_LED = 1; // -
////        RED_LED = 0;
////        FOUR_PIN = 0;
//    }
//
//    else if (current_angle == 270){
//        //  **-+
////        GREEN_LED = 0;
////        BLUE_LED = 0;
//        RED_LED = 0;
//        FOUR_PIN = 1;
//    }
//        else if ( (current_angle>0) && (current_angle<90) ){ // Coil B
//            //  +-+-
//            GREEN_LED = 1;
//            BLUE_LED = 0;
//            RED_LED = 1;
//            FOUR_PIN = 0;
//        }
//        else if ( (current_angle>90) && (current_angle<180) ){ // Coil B
//            //  -++-
//            GREEN_LED = 0; // -
//            BLUE_LED = 1; // +
//            RED_LED = 1; //+
//            FOUR_PIN = 0; //-
//        }
//        else if ( (current_angle>180) && (current_angle<270) ){
//              // -+-+
//            GREEN_LED = 0; // +
//            BLUE_LED = 1; // -
//            RED_LED = 0;
//            FOUR_PIN = 1;
//        }
//        else if ( (current_angle>270) && (current_angle<360) ){
//                // +--+
//            GREEN_LED = 1;
//            BLUE_LED = 0;
//            RED_LED = 0;
//            FOUR_PIN = 1;
//        }
//    //waitMicrosecond(STEP_TIME/STEP_SIZE);
//}
