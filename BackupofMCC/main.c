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
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/interrupt_manager.h"

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint16_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

    /*AD1CON1 Register*/
    AD1CON1bits.ADON = 1;   // enables ADC
    AD1CON1bits.FORM = 0;   // Data Output as integer form
    AD1CON1bits.SSRC = 7;   // Auto-convert set
    AD1CON1bits.ASAM = 1;   // Auto sampling enabled
    
    /*AD1CON2 Register*/
    AD1CON2bits.VCFG = 0;   // VR+ = VDD, VR- = VSS
    AD1CON2bits.SMPI = 0;   // Interrupt after every sample
    AD1CON2bits.BUFM = 0;   // One 16-word buffer
    AD1CON2bits.ALTS = 0;   // Always use MUX A input
    
    /*AD1CON3 Register*/
    AD1CON3bits.ADRC = 0;   // ADC clock uses system clock
    AD1CON3bits.SAMC = 31;  // Sample time = 31 Tad
    AD1CON3bits.ADCS = 5;   // Tad = 5Tc

    
int16_t main(void)
{
    /* Configure the oscillator for the device */
    SYSTEM_Initialize();
    /* Initialize IO ports and peripherals */
    InitApp();
    /* TODO <INSERT USER APPLICATION CODE HERE> */
   
    //int count=10;
    
    int count;
     
    
    while(1)
    {
        uint16_t tempValue[8] = {0};
        
        
        ADC1_ConversionResultBufferGet(tempValue);
        
        for (count = 0; count < 8; count++) {
            UART1_Write(tempValue[count] / 4);
            wait_ms(1);
        }
        
        UART1_Write(5);
        wait_ms(1);
        UART1_Write(5);
        wait_ms(1);
        UART1_Write(5);
        wait_ms(2);
        wait_ms(5000);
        
    }
}
