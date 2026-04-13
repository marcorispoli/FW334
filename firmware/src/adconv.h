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
    
    volatile unsigned short alarms;
#else
    #define ext extern
    #define ext_static extern
#endif

#define ALARM_MIN_VAC                       0x1
#define ALARM_OUTPUT_OVERCURRENT            0x2
#define ALARM_OUTPUT_INITIAL_OVERCURRENT    0x4

volatile ext bool ADC0Exec;

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

// Alarms Handling
ext  unsigned short  adconv_get_alarm(void);
ext  void  adconv_set_init_overcurrent_alarm(void);



#endif 
    