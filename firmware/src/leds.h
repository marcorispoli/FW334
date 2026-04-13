#ifndef _LEDS_H    
#define _LEDS_H

#include "definitions.h"  
#include "application.h"  

#undef ext
#undef ext_static

#ifdef _LEDS_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif


ext void leds_init(void);
ext void LedsControlLoop(void);

ext void led1_control(bool state, int period);
ext void led2_control(bool state, int period);
ext void led3_control(bool state, int period);
ext void led4_control(bool state, int period);
ext void led5_control(bool state, int period);
ext void ledfault_control(bool state, int period);

#endif 
    