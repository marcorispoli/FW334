#define _CONTROL_LOOP_C

#include "definitions.h"                
#include "application.h"
#include "control_loop.h"
#include "adconv.h"
#include "main.h"
#include "adconv.h"
#include "leds.h"
#include <math.h>

volatile static float Ic_actual = 0; // Corrente di controllo attuale
volatile static float Ic_instant = 0;   // Corrente di controllo istantanea


    
    
// Execution fase 
#define CONTROL_LOOP_INITIALIZATION 0
#define CONTROL_LOOP_RUNNING        1
#define CONTROL_LOOP_ALARM          2

// Global Variables
volatile static unsigned char control_loop_status = CONTROL_LOOP_INITIALIZATION;

 static void InitializationControlLoop(bool init);
 static bool AlarmControlLoop(void);


void control_init(void){
    control_loop_status = CONTROL_LOOP_INITIALIZATION;
    ControlExec = false;
    return;
}


bool AlarmControlLoop(void){
    static unsigned short alarms = 0xFFFF;
    unsigned short cur_alarms = adconv_get_alarm();
    
    // Non è cambiato lo stato degli allarmi
    if(cur_alarms == alarms){
      if(alarms) return true;
      return false;
    } 
    alarms = cur_alarms;
    
    if(alarms){        
        control_loop_status = CONTROL_LOOP_ALARM;
        
        // Azzera la corrente di controllo 
        Ic_actual = 0;
        
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
        
    // Exit Alarm commands: enter the initialization fase
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
        control_loop_status = CONTROL_LOOP_INITIALIZATION;
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
            steps = 0;
            control_loop_status = CONTROL_LOOP_RUNNING; 
            
            Ic_actual = 0;
            break;
    }
     
   
    return;
}


void ControlLoopConstantVoltage(void){
    //static bool voltage_limit_condition = false;
    
    float Ic_L;       // Corrente di controllo per eguagliare il carico
    float Ic_C;       // Corrente di controllo per compensare la perdita di energia dai condensatori    
    float Ic;         // Corrente di controllo
   
    // Acquisizione dati in ingresso
    float VOUT = adconv_get_vout();     // Tensione di uscita campionata
    float IOUT = adconv_get_iout();     // Corrente di uscita campionata
    float VAC = adconv_get_max_vac();   // Tensione VAC di picco
    
    // L'aggiornamento della corrente di controllo avviene solo in corrispondenza 
    // del minimo dalla tensione di ingresso
    if(adconv_get_vac() < adconv_get_min_vac() + 5){
        // Calcolo della corrente di controllo per compensare il carico
        Ic_L = (IOUT * VOUT * 2.2) / (VAC) ;
    
        // Calcolo della corrente di controllo per recuperare la diminuzione della tensione sui condensatori
        Ic_C = 0.2 *(TARGET_VOLTAGE*TARGET_VOLTAGE - VOUT*VOUT)/(VAC * RECOVERY_AC_CYCLES);
        if(Ic_C > MAX_RECOVERY_CURRENT) Ic_C = MAX_RECOVERY_CURRENT;
        
        // Corrente totale di controllo limitata dal valore massimo ammesso
        Ic = Ic_L + Ic_C;
        if(Ic > MAX_INPUT_CURRENT) Ic = MAX_INPUT_CURRENT;
                
        Ic_actual = Ic;
    }
    
    // In caso di Overvoltage, la tensione deve scendere sotto 
    // il valore di controllo
    /*
    if(voltage_limit_condition){
        if(VOUT < TARGET_VOLTAGE){
            voltage_limit_condition = false;
        }else Ic_actual = 0;        
    }else{
        if(VOUT >= MAX_VOUT_VOLTAGE){
            Ic_actual = 0;
            voltage_limit_condition = true;
        }
    }
    */
    if(uc_OVERVOLTAGE_Get()){
        LED_OVERCURRENT(true,0);
    }
    // per bassi valori di corrente disattiva lo switching
    if(Ic_actual == 0){
        mosfetActivation(false);
    }else{
        mosfetActivation(true);
    }
    
    // Shape della corrente di controllo
    Ic_instant = (Ic_actual * adconv_vac_fase());     
    DAC_DataWrite(DAC_CHANNEL_0,DAC(Ic_instant));  
    
    return;
}

void ControlLoopConstantCurrent(){
    static bool voltage_limit_condition = false; 
    
    // Acquisizione dati in ingresso
    float VOUT = adconv_get_vout();     // Tensione di uscita campionata
    Ic_actual = TARGET_CURRENT;
    
    
    // In caso di Overvoltage, la tensione deve scendere sotto 
    // il valore di controllo
    if(voltage_limit_condition){
        if(VOUT < TARGET_VOLTAGE){
            voltage_limit_condition = false;
        }else Ic_actual = 0;        
    }else{
        if(VOUT >= MAX_VOUT_VOLTAGE){
            Ic_actual = 0;
            voltage_limit_condition = true;
        }
    }

    // per bassi valori di corrente disattiva lo switching
    if(Ic_actual < 0.5){
        mosfetActivation(false);
    }else{
        mosfetActivation(true);
    }
    
    // Shape della corrente di controllo
    Ic_instant = (Ic_actual * adconv_vac_fase());     
    DAC_DataWrite(DAC_CHANNEL_0,DAC(Ic_instant));      
    return;
    
}

void ControlLoop(void){
     
    // Alarm condition management 
    if(AlarmControlLoop()) return;
    
    // Initialization Fase
    if(control_loop_status == CONTROL_LOOP_INITIALIZATION){ 
        InitializationControlLoop(false);
        return;
    }
           
   
#ifdef RUN_CONST_VOLTAGE
    ControlLoopConstantVoltage();
#endif
    
#ifdef RUN_CONST_CURRENT
   ControlLoopConstantCurrent();
#endif   
    
}
