#define _ADCONV_C

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "application.h"
#include "adconv.h"
#include "control_loop.h"
#include "main.h"
#include "leds.h"
#include <math.h>

static bool isVAC = true;
static bool voltage_limit_condition = false;

float adconv_get_max_vac(void){return maxVAC;}
float adconv_get_min_vac(void){return minVAC;}
float adconv_get_vac(void){return VAC;}
float adconv_get_max_vout(void){return maxVOUT;}
float adconv_get_min_vout(void){return minVOUT;}
float adconv_get_vout(void){return VOUT;}
float adconv_get_iout(void){return IOUT;}

float adconv_vac_fase(void){
    float fase;
    
    #ifdef GET_FASE_FROM_VAC
    fase = (VAC - minVAC) / (maxVAC-minVAC);    
    #endif
    
    #ifdef GET_FASE_FROM_SIN
    static int time = 0;
    static bool synch = false;
    
    // Synch
    if((!synch) && (VAC < minVAC + VAC_THRESHOLD)){
        synch = true;
        time = 0;        
    }else{
        time += AC_SMP_TIME_us;
        
        if(time >= 10 * (1000000/AC_FREQ)){ 
            time = 0;
            synch = false;
        }
        
    }

    // Sinusoide raddrizzata a 50Hz
    fase = sin(6.28 * AC_FREQ * (time+SIN_CORRECT_FASE_us)/1000000);
    if(fase < 0) fase = -fase;
    #endif
    
    #ifdef GET_FASE_FROM_CONST
        fase = GET_FASE_FROM_CONST;
    #endif

    if(fase > 1) return 1;
    if(fase < 0 ) return 0;
    return fase ;
}

unsigned short adconv_get_alarm(void){
    return alarms;        
}

void adconv_set_init_overcurrent_alarm(void){
    alarms |= ALARM_OUTPUT_INITIAL_OVERCURRENT;
}
void adconv_init(void){
        
    // INit the conversion of the AC input 
    ADC0_CallbackRegister( ADC0_Callback, 0 );
    ADC0_Enable();
    ADC0_ChannelSelect( ADC_POSINPUT_AIN5, ADC_NEGINPUT_GND); 
    ADC0_ConversionStart();
    
    isVAC= true;
    
    // INit the conversion of the Vout  
    //    ADC1_CallbackRegister( ADC1_Callback, 0 );
    ADC1_Enable();
    ADC1_ConversionStart();
    
    VAC_Initialize = false;
    vac_threshold = 10; 

    Iinput = 0;
    IOUT = 0;
    VAC = 0;
    maxVAC = 0;
    minVAC = 0;
    VOUT = 0;
    maxVOUT = 0;
    minVOUT = 0;
    
    ADC0Exec = false;
    
    // Reset alarms
    alarms = 0;
    voltage_limit_condition = false;
}


void ADC0_Callback(ADC_STATUS status, uintptr_t context){
   
    if(!(status & ADC_STATUS_RESRDY)) return;
   
    if(isVAC){
        VAC = (float) ADC0_ConversionResultGet() * 375 / 65536;
        ADC0_ChannelSelect( ADC_POSINPUT_AIN10, ADC_NEGINPUT_GND); 
        isVAC= false;
        ADC0_ConversionStart();
      
    }else{
        IOUT = (float) ((((float) ADC0_ConversionResultGet() *4.95) / 65.535) - 2500)/100.0;
        IOUT = IOUT * 860/1200;
        if(IOUT<0) IOUT = 0;
        ADC0_ChannelSelect( ADC_POSINPUT_AIN5, ADC_NEGINPUT_GND); 
        isVAC= true;
        ADC0_ConversionStart();
        
    }
    ADC0Exec = true;
}



// 116 us Conversion Time
// 86 samples in a 100 Hz
// Durata: 4.2us
#define MAXMIN_VAC_CYCLES (10 * 10000/AC_SMP_TIME_us)

void ADC0ExecProcedure(void){
    static unsigned short  cycles = MAXMIN_VAC_CYCLES;
    static float parzial_max = 0;
    static float parzial_min = 1000;
            
    WDT_Clear();   
    
    // Massimo e minimo della tensione di ingresso
    if(VAC > parzial_max){
        parzial_max = VAC;
    }

    if(VAC < parzial_min){
        parzial_min = VAC;
    }

    // Every
    cycles--;        
    if(cycles == 0){
       maxVAC =  parzial_max;
       minVAC =  parzial_min;
       if(minVAC > maxVAC) minVAC = maxVAC;
       vac_threshold = (maxVAC - minVAC) / 200;
       parzial_max = 0;
       parzial_min = 1000;
       cycles = MAXMIN_VAC_CYCLES;
    }
      
    VOUT = (float) ADC1_ConversionResultGet() *  0.0106 ;/// 1.6;
    
    // Alarm of the VAC min value
    if(maxVAC < MIN_VAC_ALARM){
        alarms |= ALARM_MIN_VAC;        
    }else if(maxVAC > MIN_VAC_RESET_ALARM){
        alarms &=~ ALARM_MIN_VAC;        
    }

    // Short Circuit
    if(IOUT > MAX_IOUT_ALARM){ 
        alarms |= ALARM_OUTPUT_OVERCURRENT;        
    }

    // Activates the Control Loop Procedures
    ControlExec = true;    
    return;
}

bool adconv_is_voltage_output_limit(void){
    
#ifdef DISABLE_OUTPUT_VOLTAGE_LIMIT
    voltage_limit_condition = false;
    return false;
#endif
    float VTH;
    if(IOUT > HARD_LOAD_CURRENT_THRESHOLD) VTH = MAX_VOUT_VOLTAGE_HARD;
    else VTH = MAX_VOUT_VOLTAGE_LIGHT;
    
    // Very High Pulse
    if(uc_OVERVOLTAGE_Get()){
        LED_OVERCURRENT(true,0);
        
    }
    
    if(voltage_limit_condition){
        if(VOUT < TARGET_VOLTAGE){
            voltage_limit_condition = false;
            
            return false;
        }
        
        return true;
    }
    
    
    if(VOUT >= VTH){           
        voltage_limit_condition = true;
        return true;
    }
    
    LED_OVERCURRENT(false,0);
    return false;

}