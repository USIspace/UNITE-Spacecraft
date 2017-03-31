/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
#include <xc.h>
#elif defined(__C30__)
#if defined(__PIC24E__)
#include <p24Exxxx.h>
#elif defined (__PIC24F__)||defined (__PIC24FK__)
#include <p24Fxxxx.h>
#elif defined(__PIC24H__)
#include <p24Hxxxx.h>
#endif
#endif

#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/pin_manager.h"
#include "adc1.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "SatelliteMode.h"

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/******************************************************************************/
/* FUNCTIONS                                               */

/******************************************************************************/




/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
int16_t main(void) {
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();

    _LATF1 = 1;

    //wait_sec(15);  // NEEDS FIXING PLZ HELP SOS
    
    //_LATF1 = 0;
    
    // Starts the CubeSat sampling in interim mode
    Satellite_Initialize();
    
    /******************************************************************************/
    /* Main Program Variable Declaration                                                  */
    /******************************************************************************/
    while (1) {
        unsigned long timer3 = TMR3_Counter16BitGet();
        unsigned long timer4 = TMR4_Counter16BitGet();
        unsigned long timer5 = TMR4_Counter16BitGet();
    };
}


    








