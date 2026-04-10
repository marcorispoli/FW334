#ifndef _MAIN_H    
#define _MAIN_H

#include "definitions.h"  
#include "application.h"  

#undef ext
#undef ext_static

#ifdef _MAIN_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

// Execution fase 
#define MAIN_FASE_INIT_LOOP 0
#define MAIN_FASE_RUNNING_LOOP 1

// Global Variables
volatile ext unsigned char main_fase;


ext void mosfetActivation(bool state);
ext void outputActivation(bool state);


#endif 
    