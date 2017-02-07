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
//This finds the index of the last stored value in array of length elements
int Find(int *arr[], int length) { 
    int i;

    while ((arr[i] != 0) && (i < length)) {
        i++;
    }
    return i;
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
    uint16_t count, channel;
    uint8_t SamplePackage[8]; //This is the building of the package from the ADC data
    int16_t Send = 0;
    uint8_t Package[36];
    int *Storage[4]; //Can hold 4 packages 
    int Location = 0;
    int i; //Index variables 
    int j; //Index variables 
    int Keep = 8;
    //This sets the initial array to zero by a for loop
    for (i = 0; i < 36; i++) {
        Package[i] = 0;
        wait_ms(1);
    }

    /******************************************************************************/
    /* MAIN LOOP HERE                                              */
    /******************************************************************************/

    while (1) {

        /*This samples the data and places the values into the package array
         * so this code polls the data and builds the package at the same time
      this in the future code will build a larger 36 byte package that can be
      send to the arduino once the busy line is in the non busy stage - colin*/
        for (j = 0; j < 4; j++) {
            for (count = 0; count < 8; count++) {
                _LATF0 = 1; //Purple LED when it is polling data
                channel = (count + 8); // Increment ADC channel
                SamplePackage[count] = (ADC1_ResultGetFromChannel(channel) / 4); //Get ADC value from channel

            }
            
            // Store a sweep of 8 ADC values in Package
            for (count = 0; count < 8; count++) {
                Package[count + 8*j] = SamplePackage[count % 8]; 
            }
        }
        
        // Fill last four spots in package
        Package[32] = 0; 
        Package[33] = 0;
        Package[34] = 0;
        Package[35] = 0;

        wait_ms(5000);
        
        /*Location =Find(Storage); //Finds the location of the last saved data.
        int j=0;
        
        for (i = Location;i<=Location+8;Location++){
            Storage[Location] = SamplePackage[j];
            j++;
    }
         * */ //Soon to be implemented
        
        
        if (!PORTEbits.RE5) {
            _LATF0 = 0;
            //Send 'Package'
            
            for (i = 0; i < 36; i++) {
                UART1_Write(Package[i]);
                wait_ms(1);
            }
            //check if more data - send while more
            //When done send a finished data package 
            UART1_Write(111);
            wait_ms(1000);
            //Wait till arduino switches its ready singal
        } else {
            
            // 
            i = Find(Storage, 4);
            
            Storage[i] = Package;
        }
        
        
        
    }

}





