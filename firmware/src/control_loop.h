#ifndef _CONTROL_LOOP_H    
#define _CONTROL_LOOP_H

#include "definitions.h"  
#include "application.h"  

#undef ext
#undef ext_static

#ifdef _CONTROL_LOOP_C
    #define ext
    #define ext_static static 

#else
    #define ext extern
    #define ext_static extern
#endif

ext bool ControlExec;

ext void control_init(void);
ext void ControlLoop(void);
#endif 
    