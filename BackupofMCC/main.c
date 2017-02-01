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

/* i.e. uint16_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/

int16_t main(void) {
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();
    /* TODO <INSERT USER APPLICATION CODE HERE> */

    //int count=10;

    uint16_t count, channel;
    char package[12]; //This is the building of the package from the ADC data
    int16_t Send = 0;
   

    int i; //Index variables 
    int j; //Index variables 
    
    //This sets the intial array to zero by a For loop
    for (i = 0; i < 12; i++) {
        package[i] = 0; 
        wait_ms(1);
    }
    
    while (1) {
        /*This samples the data and places the values into the package array
         * so this code polls the data and builds the package at the same time
         this in the future code will build a larger 36 byte package that can be
         send to the arduino once the busy line is in the non busy stage - colin*/
        for (count = 0; count < 8; count++) {
            _LATF0 = 1; //Purple LED
            channel = count + 8; // Increment ADC channel
            package[count + 2] = (ADC1_ResultGetFromChannel(channel)/4);
             
        }
        //This is the buffer to fill the rest of the array 
        if (package[0] == 0) {
            package[0] = 212;
            package[1] = 212;
            package[10] = 212;
            package[11] = 212;
        }
        UART1_Write(1); //Preamble
        UART1_Write(1);//Preamble
        UART1_Write(1);//Preamble 
        
        /* This here sends the package. Given that UART1_Write() is a function
         passing a array through it is not not possible, it will later need to be
         in pointer form, this was easier to code at the time - colin*/
        for (j = 0; j < 12; j++) {
            Send = package[j];
            UART1_Write(Send);
            wait_ms(1);
        }
        /* Again this is rewriting the array for transmission to be all zeros,
         for some reason package[] = {0} hated me - colin*/
        for (i = 0; i < 12; i++) {
            package[i] = 0;
            wait_ms(1);
        }
    }


    _LATF0 = 0; //Purple LED
}



//ADCValue[0] = ADC1_ResultGetFromChannel(8);
//UART1_Write(ADCValue[0]/4);


/* PIC24 documentation sample code */
/* 
int i, conversion;
//ADC1_Initialize();
ADC1_ChannelSelect(8);
ADC1_Start();
//Provide Delay
for(i=0;i <1000;i++)
{
}
ADC1_Stop();
while(!ADC1_IsConversionComplete())
{
    //ADC1_Tasks();   
}
conversion = ADC1_ConversionResultGet() / 4;
 */







