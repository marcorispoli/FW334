/*!
 * \defgroup  applicationModule Application Modules section
 * 
 */


/*!
 * \defgroup  applicationConstants Application Constants section
 * 
 * \ingroup applicationModule
 */


/*!
 * \defgroup  libraryModules Library Modules Used in the Application.
 */


/*!
 * \mainpage
 *
 * # Overview
 *
 * This Firmware implements the functions of the PCB/22-303 device  software specification.
 * 
 * 
 * # CONFIGURATION SETTING
 * 
 * ## BOOTLOADER object link
 * 
 * This application links the FW303_BOOT.xx.yy.hex as a loadable object;
 * 
 * ## SHARED RAM AREA DEFINITION
 * 
 * The Application reserves 16 byte of RAM for the Bootloader shared area
 * 
 * ```text
 * Menu-Production
 * - Set Project Configuration
 *    - Customize ..
 *        - XC32 Global Options
 *            - XC32-ld
 *                - Additional options: -DRAM_ORIGIN=0x20000010,-DRAM_LENGTH=0x3FFF0 
 * 
 *   ```
 * 
 * 
 * ## HARMONY 3 REVISION PACK 
 * 
 * The Application has been developed with the use of the Microchip Harmony 3
 * libraries with the following revision codes: (see config/default/harmony-manifest-success.yml)
 * 
 *  - mcc_version: v5.3.0
 *  - mcc_core_version: v5.5.0
 *  - mplabx_version: v6.05        # if MPLAB X plugin only
 *  - harmony_version: v1.3.0
 *  - compiler: XC32 (v3.01) 
 *
 *  - modules:
 *       - {name: "csp", version: "v3.9.1"}
 *       - {name: "dev_packs", version: "v3.9.0"}
 *
 * ## PERIPHERAL CONFIGURATION
 * 
 * ### CLOCK GENERATOR CONFIGURATION
 * 
 * + DFLL: set to 48MHz Open Loop Mode;
 * + GLK1: input = DFLL, output = 48MHz;
 * + FDPLL0: input = GLK1, output = 120MHz;
 * + GLK0: input = FDPLL0, output = 120MHz;
 * + GLK2: input = DFLL, output = 8MHz;
 * + GLK4: input = DFLL, output = 24MHz;
 * 
 * ### PERIPHERAL CLOCK CONFIGURATION
 * 
 * + CPU : input = GLK0, output = 120MHz;
 * + CAN0: input = GLK4; output = 24MHz;
 * + TCO,TC1 : input = GLK2, output = 8MHz;
 * 
 *  ### MCC SYSTEM configuration
 * 
 * ```text
 * Device & Project Configuration
 *    - Project Configuration
 *        - Tool ChainSelections
 *            - XC32 Global Options
 *                - Linker
 *                    - Symbols & Macros
 *                        - Application Start Address (Hex) = 0x2000
 * 
 * Device & Project Configuration
 *      - ATSAME51J20A Device Configuration/Fuse settings:
 *              - Number Of Phisical NVM Blocks Composing a SmartEEPROM Sector = 1;
 *              - Size Of SmartEEPROM Page = 1;
 * 
 * ```
 * 
 * ### MCC CAN0 configuration
 * 
 * 
 * + CAN Operational Mode = NORMAL;
 * + Interrupt Mode: Yes;
 * + Bit Timing Calculation
 *  + Nominal Bit Timing
 *      + Automatic Nominal Bit Timing: Yes;
 *      + BIt Rate: 1000
 * 
 * + Use RX FIFO 0: Yes
 *   + RX FIFO 0 Setting
 *      + Number of element: 1
 * 
 * + Use RX FIFO 1: Yes
 *   + RX FIFO 1 Setting
 *      + Number of element: 1
 * 
 * + Standard Filters 
 *  + Number Of STandard Filters: 2
 * 
 *  + Standard Filter 1
 *      + Type: Range;
 *      + ID1: 0x140 + Device ID 
 *      + ID2: 0x140 + Device ID 
 *      + Element Configuration: Store in RX FIFO 0
 * 
 *  + Standard Filter 2
 *      + Type: Range;
 *      + ID1: 0x100 + Device ID  
 *      + ID2: 0x100 + Device ID 
 *      + Element Configuration: Store in RX FIFO 1
 * 
 *  + Reject Standard Remote Frames: YES
 * 
 *  + Timestamp Enable: YES 
 * 
 * ### TC0 CONFIGURATION
 * 
 *  
 * ### AC0 CONFIGURATION
 * 
 * + Peripheral Clock divide: 4
 * + Sample Lenght: 32
 * + Conversion Time: 4us
 * + Select Reference: VDDANA
 * + Select Conversion Trigger: SW Trigger 
 * + Select Result Resolution : For Averaging mode output
 * + Accumulated Samples: 16
 * + Left Aligned Result
 * + Enable Result Ready Interrupt
 * 
 * ### DAC CHANNEL 0 CONFIGURATION
 * 
 * + Select DAC Output Mode: Single-Ended Output
 * + Reference Selection for DAC: Analog Supply
 * + Enable DAC channel 0
 * + Conversione Speed: 1MSPS
 * +DAC Data Register Adjustment: LEFT-ADJUSTED
 * + Run in Standby: yes
 * + Oversampling: No Oversampling
 * + Refresh Rate: Do no refresh
 * 
 * # Licensing
 *
 * The Application has been developed with the Microchip Harmony 3 libraries 
 * with the Microchip License Agreement conditions as described in:
 * - @ref licenseAgreement 
 *
 * # Application documentation
 * 
 * This documentation has been produced with Doxygen 
 */



#ifndef _APPLICATION_H    /* Guard against multiple inclusion */
#define _APPLICATION_H

// Defines the Application revision code

/** \addtogroup applicationConstants
 *  @{
*/

// Application Revision 
static const unsigned char  APPLICATION_MAJ_REV =  1 ;  //!< Revision Major Number
static const unsigned char  APPLICATION_MIN_REV =  0 ;  //!< Revision Minor Number
static const unsigned char  APPLICATION_SUB_REV =  1 ;  //!< Revision build Number

/** @}*/
//___________________ SELEZIONE MODULI _____________________
//#define CAN_BUS_ACTIVATION      // Attivazione Can Bus
//#define MOSFET_SWITCH_ENABLED   // Attivazione Switching Mosfets
#define OUTPUT_SWITCH_ENABLED   // Attivazione Output Mosfets

//___________________ SELEZIONE FUNZIONAMENTO _____________________
//#define TEST_SWITCH
//#define TEST_REFERENCE 
//#define RUN_CONST_CURRENT   // Corrente Fissa
#define RUN_CONST_VOLTAGE // Regolazione tensione di uscita


//___________________ SELEZIONE FASE _____________________
#define GET_FASE_FROM_VAC
//#define GET_FASE_FROM_SIN
//#define GET_FASE_FROM_CONST 1

//___________________ IMPOSTAZIONE DAC _____________________
#define SENS_VREF 0.33       // Volt
#define SENS_ISENSE 75     // mV/A
#define DAC_REFERENCE 3.3   // Impostazione uC-DAC

#define MAX_SENS_CURRENT (SENS_VREF*1000/SENS_ISENSE)
#define DAC(I) (unsigned short) (I * SENS_ISENSE * 65.535 / DAC_REFERENCE)
 
//___________________ PERFORMANCES _____________________
#define VAC_INPUT (220*1.4)    
#define VAC_THRESHOLD  (vac_threshold) // 5

#define AC_SMP_TIME_us 120
#define AC_FREQ  50
#define SIN_CORRECT_FASE_us 650

// target di tensione e corrente
#define TARGET_VOLTAGE 560
#define MAX_CURRENT 5

#define MIN_VAC_ALARM (VAC_INPUT*0.5)
#define MIN_VAC_RESET_ALARM (VAC_INPUT*0.7)

#define INITIALIZATION_LOOP_MAX_CURRENT 2
#define MAX_IOUT_ALARM 20
#endif 