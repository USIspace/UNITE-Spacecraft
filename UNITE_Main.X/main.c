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

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/******************************************************************************/
/* FUNCTIONS                                               */

/******************************************************************************/


void StartWaitTimer() {
    
    // Begins wait timer
    TMR1_Initialize();
    TMR1_Start();
    
    // Initialize Debug light to on
   _LATE1 = LED_ON;
   
   _LATE2 = LED_OFF;
   _LATE3 = LED_OFF;
   _LATE4 = LED_OFF;
   
   _RG9 = 1;                   //Slave select 1
}

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
int16_t main(void) {
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();
    
//    ConfigureOscillator();

    StartWaitTimer();

    while (1);
}


    








