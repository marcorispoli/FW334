#ifndef _ADCONV_H    
#define _ADCONV_H

#include "definitions.h"  
#include "application.h"  


#undef ext
#undef ext_static

#ifdef _ADCONV_C
    #define ext
    #define ext_static static 

    int alarm_vac_count;
    bool  VAC_Initialize;
    float vac_threshold; 
    volatile  float Iinput;
    volatile  float IOUT;

    volatile  float VAC;
    volatile  float maxVAC;
    volatile  float minVAC;

    volatile  float VOUT;
    volatile  float maxVOUT;
    volatile  float minVOUT;

#else
    #define ext extern
    #define ext_static extern
#endif



volatile ext bool alarm_VAC;
volatile ext bool ADC0Exec;
volatile ext bool alarm_overcurrent;

ext void adconv_init(void);
ext void ADC0ExecProcedure(void);
ext void ADC0_Callback(ADC_STATUS status, uintptr_t context);

ext  float adconv_get_max_vac(void);
ext  float adconv_get_min_vac(void);
ext  float adconv_get_vac(void);
ext  float adconv_get_max_vout(void);
ext  float adconv_get_min_vout(void);
ext  float adconv_get_vout(void);
ext  float adconv_get_iout(void);
ext  float adconv_vac_fase(void);
ext  bool adconv_get_alarm(void);

#endif 
    