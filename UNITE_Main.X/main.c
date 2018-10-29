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
#include <xc.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include <libpic30.h>
#include "user.h"          
#include "mcc_generated_files/mcc.h"
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
int16_t main(void) {
    
    __delay_ms(1000);
    
    /* Configure the oscillator for the device */
    /* Initialize IO ports and peripherals */
    SYSTEM_Initialize();
    
    // Wait for EPS startup to complete
    while(IsLineBusy(SimplexUnit)) ClrWdt();
    
    // Initialize Satellite configurations and main loop Timer
    Satellite_Initialize();
     
    // Endless loop to prevent main() from returning and ending the program
    while (1) ClrWdt();
}


    








