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
int Find(uint8_t arr[]) { //This finds the value where the last stored value is
    int i;
    int count;

    while (arr[i] == 0) {
        count++;
    }
    return count;
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
    char SamplePackage[8]; //This is the building of the package from the ADC data
    int16_t Send = 0;
    char Package[36];
    char Storage[128]; //Can hold 16 sample packages 
    int Location = 0;
    int i; //Index variables 
    int j; //Index variables 
    int Keep = 8;
    //This sets the initial array to zero by a for loop
    for (i = 0; i < 12; i++) {
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

        while (PORTEbits.RE5) {
            for (count = 0; count < 8; count++) {
                _LATF0 = 1; //Purple LED when it is polling data
                channel = count + 8; // Increment ADC channel
                SamplePackage[count] = (ADC1_ResultGetFromChannel(channel) / 4);
            }
            wait_ms(5000);
        }
        /*Location =Find(Storage); //Finds the location of the last saved data.
        int j=0;
        
        for (i = Location;i<=Location+8;Location++){
            Storage[Location] = SamplePackage[j];
            j++;
        }
         * */ //Soon to be implemented

        while (!PORTEbits.RE5) {
            _LATF0 = 0;
            //Send 'Package'
            for (i = 0; i < 8; i++) {
                UART1_Write(SamplePackage[i]);
            }
            //check if more data - send while more
            //When done send a finished data package 
            UART1_Write(111);
            wait_ms(1000);
            //Wait till arduino switches its ready singal
        }


    }

}





