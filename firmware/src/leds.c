#define _MAIN_C

#include "definitions.h"                // SYS function prototypes
#include "application.h"
#include "leds.h"

// led control routines
#define LED1ID 0x1
#define LED2ID 0x2
#define LED3ID 0x4
#define LED4ID 0x8
#define LED5ID 0x10
#define LEDFAULTID 0x20

static int led1_period = 0;
static int led2_period = 0;
static int led3_period = 0;
static int led4_period = 0;
static int led5_period = 0;
static int ledfault_period = 0;
static int led1_counter = 0;
static int led2_counter = 0;
static int led3_counter = 0;
static int led4_counter = 0;
static int led5_counter = 0;
static int ledfault_counter = 0;
static bool led1_status = false;
static bool led2_status = false;
static bool led3_status = false;
static bool led4_status = false;
static bool led5_status = false;
static bool ledfault_status = false;

void leds_init(void){

    // LED initialization
    LED1_Clear(); // Assigned to Switching Activation Status
    LED2_Clear(); // Assigned to Output-Switch Activation Status
    LED4_Clear(); 
    LED3_Clear(); 
    LED5_Clear(); // Assigned to Initialization Loop
    LED_FAULT_Clear(); // Assigned to Fault Conditions
    
    led1_counter = 0;
    led2_counter = 0;
    led3_counter = 0;
    led4_counter = 0;
    led5_counter = 0;
    ledfault_counter = 0;
    
    led1_status = false;
    led2_status = false;
    led3_status = false;
    led4_status = false;
    led5_status = false;
    ledfault_status = false;
    
}

// Executed every 15ms
void LedsControlLoop(void){
    
    // LED1 Routine
    if((led1_status) && (led1_period)){
        led1_counter++;
        if(led1_counter >= led1_period){ 
            led1_counter = 0;
            LED1_Toggle();
        }
    }
    
    // LED2 Routine
    if((led2_status) && (led2_period)){
        led2_counter++;
        if(led2_counter >= led2_period){ 
            led2_counter = 0;
            LED2_Toggle();
        }
    }

    // LED3 Routine
    if((led3_status ) && (led3_period)){
        led3_counter++;
        if(led3_counter >= led3_period){ 
            led3_counter = 0;
            LED3_Toggle();
        }
    }
    
    // LED4 Routine
    if((led4_status) && (led4_period)){
        led4_counter++;
        if(led4_counter >= led4_period){ 
            led4_counter = 0;
            LED4_Toggle();
        }
    }

    // LED5 Routine
    if((led5_status) && (led5_period)){
        led5_counter++;
        if(led5_counter >= led5_period){ 
            led5_counter = 0;
            LED5_Toggle();
        }
    }
    
    // LEDFAULT Routine
    if((ledfault_status) && (ledfault_period)){
        ledfault_counter++;
        if(ledfault_counter >= ledfault_period){ 
            ledfault_counter = 0;
            LED_FAULT_Toggle();
        }
    }

}

void led1_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == led1_status) led1_period = period;
    led1_period = period;
    led1_counter = 0;
    if(!state) LED1_Clear();
    else LED1_Set();    
    led1_status = state;
}

void led2_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == led2_status) led2_period = period;
    led2_period = period;
    led2_counter = 0;
    if(!state) LED2_Clear();
    else LED2_Set();    
    led2_status = state;
}

void led3_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == led3_status) led3_period = period;
    led3_period = period;
    led3_counter = 0;
    if(!state) LED3_Clear();
    else LED3_Set();    
    led3_status = state;
}

void led4_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == led4_status) led4_period = period;
    led4_period = period;
    led4_counter = 0;
    if(!state) LED4_Clear();
    else LED4_Set();    
    led4_status = state;
}

void led5_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == led5_status) led5_period = period;
    led5_period = period;
    led5_counter = 0;
    if(!state) LED5_Clear();
    else LED5_Set();    
    led5_status = state;
}

void ledfault_control(bool state, int period){
    
    // if the status is not change, changes only the period
    if(state == ledfault_status) ledfault_period = period;
    ledfault_period = period;
    ledfault_counter = 0;
    if(!state) LED_FAULT_Clear();
    else LED_FAULT_Set();    
    ledfault_status = state;
}


/** @}*/
/*******************************************************************************
 End of File
*/

