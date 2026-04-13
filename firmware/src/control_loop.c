#define _CONTROL_LOOP_C

#include "definitions.h"                
#include "application.h"
#include "control_loop.h"
#include "adconv.h"
#include "main.h"
#include "adconv.h"
#include "leds.h"
#include <math.h>

 static void InitializationControlLoop(bool init);
 static bool AlarmControlLoop(void);


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


bool AlarmControlLoop(void){
    static unsigned short alarms = 0xFFFF;
    unsigned short cur_alarms = adconv_get_alarm();
    
    if(cur_alarms == alarms){
      if(alarms) return true;
      return false;
    } 
    alarms = cur_alarms;
    
    if(alarms){
        // Spegne tutto!
        outputActivation(false);
        mosfetActivation(false);

        if(alarms & ALARM_MIN_VAC){
            LED_ALARM(true,0);
        }else if(alarms & ALARM_OUTPUT_OVERCURRENT){
            LED_ALARM(true,10);
        }else if(alarms & ALARM_OUTPUT_INITIAL_OVERCURRENT){
            LED_ALARM(true,50);
        }else LED_ALARM(true,100);

        return true;
    }
        
    // Exit Alarm commands
    LED_ALARM(false,0);
    InitializationControlLoop(true); // Reset procedures

    return false;
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
void InitializationControlLoop(bool init){
    // This routine executes every about 120us
    static unsigned int steps = 0;
    static int counter = 0;
    
    if(init){
        // Reset procedure on request
        steps = 0;
        counter = 0;
        LED_INITIALIZATION(false,0);
        return;
    }
    
    switch(steps){
        case 0:
            // Output Off
            outputActivation(false);
            mosfetActivation(false);
            steps++;
            LED_INITIALIZATION(true,0);
            counter = 0;
            break;
            
        case 1: // Waiting Vout stabilized to the VAC max value
            if(adconv_get_vout() < MIN_VAC_RESET_ALARM){ 
                counter = 0;
                return;            
            }
            counter++;
            if(counter > 500) steps++;            
            break;
        
        case 2: // Open the Output mosfet to read the Load            
            outputActivation(true);
            counter = 0;
            steps++;
            break;
            
        case 3: // read the Load before to run the control Loop
            if(adconv_get_iout() > INITIALIZATION_LOOP_MAX_CURRENT){
                
                // Va in allarme avendo assorbito troppa corrente da subito
                outputActivation(false);
                adconv_set_init_overcurrent_alarm();
                steps = 0;
                LED_INITIALIZATION(false,0);
                return;
            }
            counter++;
            if(counter > 500) steps++;
            break;
            
        case 4:
            // The Output is stable and the Load is not heavy
            // The Initialization Loop can finish here!
            LED_INITIALIZATION(false,0);
            return;
            break;
    }
     
    // legge la tensione in uscita ed attende che il valore si 
    // stabilizzi al picco della tensione di rete
    
    
   
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
    if(AlarmControlLoop()) return;
    
    // Initialization Fase
    if(main_fase == MAIN_FASE_INIT_LOOP){ 
        InitializationControlLoop(false);
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
    
}
