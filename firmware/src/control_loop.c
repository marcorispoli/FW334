#define _CONTROL_LOOP_C

#include "definitions.h"                
#include "application.h"
#include "control_loop.h"
#include "adconv.h"
#include "main.h"
#include "adconv.h"
#include <math.h>

void control_init(void){
    ControlExec = false;
    DACVAL = 0;
    target = false;
    DAC_IREF = 0;
    Iref = 0;
    Ifollow = 0;
    output_voltage_level = 0;
    DAC_DataWrite(DAC_CHANNEL_0,0); 
    return;
}

void AlarmControlLoop(void){
    outputActivation(false);
    mosfetActivation(false);
}

/**
 * Voltage Output Stabilization Procedure
 * 
 * After a system Power Up or whenever a fault in the
 * AC input should happen, the Output switch is Disabled.
 * 
 * The Voltage on the Capacitors is measured and when 
 * the rated voltage is reached, the Output Switch is enabled
 * and the Load is measured.
 * 
 * In case the Load should be OK (no shorts or heavy loads)
 * then the initial loop can terminate and the normal control 
 * can start.
 *  
 */
void InitializationControlLoop(void){
    
    // Output Off
    outputActivation(false);
    mosfetActivation(false);
     
    
   
    return;
}

#ifdef GET_FASE_FROM_SIN
static  float getFaseFromFunc(void){
    float fase = 0;
    static int time = 0;
    static bool synch = false;
    
    
    // Synch
    if((!synch) && (VAC < minVAC+VAC_THRESHOLD)){
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
    return fase;
}
#endif



#ifdef TEST_REFERENCE
static void ExecTestReference(void){
    float fase = 0;
    float Ifollow = 0;
    static int time = 0; // useconds
    
    
    uc_SHDOWN_Clear();
    time += AC_SMP_TIME_us;
    if(time >= 50000){ 
       time = 0;
    }
   

    // Sinusoide raddrizzata a 50Hz
    fase = sin(6.28 * 50 * time/1000000);
    if(fase < 0) fase = -fase;
    Ifollow = (MAX_CURRENT * fase);  
    unsigned short DACVAL = DAC(Ifollow);
    
    DAC_DataWrite(DAC_CHANNEL_0,DACVAL);  
    return;
}
#endif


void ControlLoop(void){
    static float fase = 0;
    
     #ifdef TEST_SWITCH
        LED_FAULT_Clear();
        DAC_DataWrite(DAC_CHANNEL_0,65535);
        return;
    #endif

    #ifdef TEST_REFERENCE
        LED_FAULT_Clear();
        ExecTestReference();
        return;
    #endif
    
    // Alarm condition management
    if(adconv_get_alarm()){ 
        LED_FAULT_Set();                      
        AlarmControlLoop();
        return;
    }else LED_FAULT_Clear();
    
    // Initialization Fase
    if(main_fase == MAIN_FASE_INIT_LOOP){ 
        InitializationControlLoop();
        return;
    }
           
    // Normal Control
   
#ifdef RUN_CONST_VOLTAGE
    static float Inom = 0;
    bool update = false;
    float VOUT = adconv_get_vout();
    float IOUT = adconv_get_iout();
    
    
    float ival = 1.41 * (IOUT * VOUT) / (adconv_get_max_vac()) ;
    
    if(ival < Inom * 0.7) update = true;
    else if(ival > Inom * 1.3) update = true;
    Inom = ival;
    if(VOUT < TARGET_VOLTAGE-50) update = true;
    if(VOUT > TARGET_VOLTAGE + 30 ) update = true;
    if(adconv_get_vac() < adconv_get_min_vac() + 5) update = true;
    
    
    if(VOUT < TARGET_VOLTAGE-100){
        output_voltage_level = 0;
        Inom = 3*Inom;
    }else if(VOUT < TARGET_VOLTAGE-50){
        output_voltage_level = 1;
        Inom = 3*Inom;//Inom = 2.2*Inom;
    }else if(VOUT < TARGET_VOLTAGE-20){
        output_voltage_level = 2;
        Inom = 3*Inom;//Inom = 1.8*Inom;
    }else if(VOUT < TARGET_VOLTAGE-10){
        output_voltage_level = 3;
        Inom = 2*Inom;//Inom = 1.4*Inom;
    }else if(VOUT < TARGET_VOLTAGE) {
        output_voltage_level = 5;
        Inom = 1.8*Inom;//Inom = Inom*1.2;
    }else if(VOUT < TARGET_VOLTAGE+10) {
        output_voltage_level = 5;
        Inom = Inom;
    }else if(VOUT < TARGET_VOLTAGE+20) {
        output_voltage_level = 5;
        Inom = 0.8 *Inom;
    }else if(VOUT < TARGET_VOLTAGE+30) {
        output_voltage_level = 5;
        Inom = 0.6 *Inom;
    }else if(VOUT > TARGET_VOLTAGE+30) {
        output_voltage_level = 6;
        Inom = 0.3 *Inom;
    }
    
    if(update){
        Iref = Inom;        
    }

    if(VOUT > 600) Iref= 0;
    
    // Massima corrente accettabile
    if(Iref >MAX_CURRENT) Iref = MAX_CURRENT;
#endif 

#ifdef RUN_CONST_CURRENT
    Iref = MAX_CURRENT;
#endif   
             
    
    #ifdef GET_FASE_FROM_VAC
        fase = adconv_vac_fase();
    #endif
    #ifdef GET_FASE_FROM_SIN
        fase = getFaseFromFunc();
    #endif
    #ifdef GET_FASE_FROM_CONST
        fase = GET_FASE_FROM_CONST;
    #endif


    // Shape della corrente
    Ifollow = (Iref * fase);     
    DACVAL = DAC(Ifollow);
    DAC_DataWrite(DAC_CHANNEL_0,DACVAL);  
    
     // Se la tensione massima di ingresso è bassa interrompe l'inseguimento
    if(alarm_VAC)
    {
        mosfetActivation(false);
        return;
    }else{
        mosfetActivation(true);
    }
 
}
