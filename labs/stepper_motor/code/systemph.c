/*
 * systemph.c
 *
 *  Created on: Sep 1, 2024
 *      Author: clara
 */
#include "systemph.h"
#include "uart0.h"

void yield(){};
void kbhit(){};
void pibool(bool on){
    if(on){
        putsUart0("pi on\n");
    }
    else{
        putsUart0("pi off\n");
    }
};
void preempt(bool on){
    if(on){
        putsUart0("preempt on\n");
    }
    else{
        putsUart0("preempt off\n");
    }
};
void sched(bool prio_on){
    if(prio_on){
        putsUart0("sched prio\n");
    }
    else{
        putsUart0("sched rr\n");
    }
};
void ps(){
    putsUart0("ps called\n"); // Make a function
};
void ipcs(){
    putsUart0("ipcs called\n"); // Make a function
};
void pkill(char * process){
    putsUart0(process);
}
void pidof(const char name[]){
    putsUart0(name);
    putsUart0(" launched\n");
};
void kill(uint32_t pid){
    putiUart0(pid);
    putsUart0(" killed\n");
};

