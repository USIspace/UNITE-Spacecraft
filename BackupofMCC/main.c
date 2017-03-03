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

void Clear(int *buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}

void CheckForModeUpdate(int time) {
    
    /*
     Debugging only
    UART1_Write(0);
    UART1_Write(time);
    UART1_Write(0);
    */
    
    if ((time == 21600) || ((currentMode == interim) && (time > 21600))) {
        shouldChangeMode = true;
    } else if ((time == 36000) || ((currentMode == science) && (time > 36000))) {
        shouldChangeMode = true;
    } else if ((time == 40500) || ((currentMode == reentry) && (time > 40500))) {
        shouldChangeMode = true;
    }
}
/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
int16_t main(void) {
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();


    /******************************************************************************/
    /* Main Program Variable Declaration                                                  */
    /******************************************************************************/
    int SamplePackage[8]; //This is the building of the package from the ADC data
    const int ARRAY_SIZE = 8;

    int timeCount = 0;
    /******************************************************************************/
    /* MAIN LOOP HERE                                              */
    /******************************************************************************/
    int wait;
    // Wait for 2700 cycles of 1 second
    for (wait = 0; wait < 2700; wait++) {
        wait_ms(1000);
    }
    
    while (currentMode != safe) {

        CheckForModeUpdate(timeCount);
        
        Clear(SamplePackage, ARRAY_SIZE);

        GetTempData(SamplePackage, ARRAY_SIZE);
        SendData(SamplePackage, ARRAY_SIZE);

        currentMode = UpdateMode();
        wait_ms(DelayForMode());

        UART1_Write(111);
        //check if more data - send while more
        //When done send a finished data package 


        timeCount = timeCount + (DelayForMode() / 1000);
    }

}







