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
#include "adc1.h"


void InitializeADC1(void) {

    
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


}

uint16_t ADC1_ResultGet(uint16_t *buffer)
{
    int count;
    
    for (count = 8; count <= 15; count++) {
        _CH0SA = count;
    
        _SAMP = 1;
        while (!_DONE) {};
        
        buffer[count - 8] = ADC1BUF0;
        
        _SAMP = 0;
    }
    
    return count;
}

uint16_t ADC1_ResultGetFromChannel(uint16_t channel) {
    
    _CH0SA = channel;
    
    _SAMP = 1;
    while (!_DONE) {};
    _SAMP = 0;

    return ADC1BUF0;
        
}