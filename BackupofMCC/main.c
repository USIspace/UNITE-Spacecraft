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
 
int16_t main(void)
{
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();
    /* TODO <INSERT USER APPLICATION CODE HERE> */
   
    //int count=10;
    
    uint16_t count, channel;
        
    while(1)
    {
        
        uint16_t ADCValue[8] = { 0 };
        
        // Iterates over each active ADC Channel from 8-15 and outputs results
        for (count = 0; count < 8; count++) {
            channel = count + 8;                                    // Increment ADC channel
            ADCValue[count] = ADC1_ResultGetFromChannel(channel);   // Get ADC Conversion Result for 'channel'
            UART1_Write(ADCValue[count] / 4);                       // Output the shortened result to Arduino
            wait_ms(1);
        }
        
        
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
        
        /* Simulate Idle line */
        UART1_Write(0);
        wait_ms(1);
        UART1_Write(0);
        wait_ms(1);
        UART1_Write(0);

        wait_ms(5000);
        
    }
}
