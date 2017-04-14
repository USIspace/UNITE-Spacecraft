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


//Used as a way to index and enumerate over the hex enum ADC_CHANNEL 
const ADC_CHANNEL ADC_CHANNEL_MAP[16] =
{
    ADC_CHANNEL_VBG_2,  // First two values used as 
    ADC_CHANNEL_VBG,    // filler so index matches channel (index of 2 == AN2)
    ADC_CHANNEL_AN2,
    ADC_CHANNEL_AN3,
    ADC_CHANNEL_AN4,
    ADC_CHANNEL_AN5,
    ADC_CHANNEL_AN6,
    ADC_CHANNEL_AN7,
    ADC_CHANNEL_AN8,
    ADC_CHANNEL_AN9,
    ADC_CHANNEL_AN10,
    ADC_CHANNEL_AN11,
    ADC_CHANNEL_AN12,
    ADC_CHANNEL_AN13,
    ADC_CHANNEL_AN14,
    ADC_CHANNEL_AN15
};


void InitializeADC1(void) {

    
    /*AD1CON1 Register*/
    AD1CON1 = 0;            // Don't enable ADC on initialize
    
    /*
    AD1CON1bits.DONE = 0;   // Allows conversion to begin
    AD1CON1bits.FORM = 0;   // Data Output as integer form
    AD1CON1bits.SSRC = 0;   // Begin conversion once SAMP is set to 0; For auto-convert set to (7)
    AD1CON1bits.ASAM = 0;   // Auto sampling disabled
    */
    
    /*AD1CON2 Register*/
    AD1CON2 = 0x00F0;
    /*
    AD1CON2bits.VCFG = 0;   // VR+ = VDD, VR- = VSS
    AD1CON2bits.SMPI = 0;   // Interrupt after every sample
    AD1CON2bits.BUFM = 0;   // One 16-word buffer
    AD1CON2bits.ALTS = 0;   // Always use MUX A input
    */
    
    /*AD1CON3 Register*/
    AD1CON3 = 0x0002;       // Manual Sample; Tad = 3Tcy    
    /*
    AD1CON3bits.ADRC = 0;   // ADC clock uses system clock
    AD1CON3bits.SAMC = 31;  // Sample time = 31 Tad
    AD1CON3bits.ADCS = 5;   // Tad = 5Tc
    */
    
    AD1PCFG = 0x00FF;       // Sets AN8 - AN15 as analog inputs (Must set to 0 for analog input)
    AD1CSSL = 0;            
    

}

/* Not functioning yet...needs to be updated to return an array of buffer values*/
uint16_t ADC1_ResultGet(uint16_t *buffer)
{
    int count;
    
    for (count = 8; count < ADC_MAX_CHANNEL_COUNT; count++) {
    
        buffer[count - 8] = ADC1_ResultGetFromChannel(count);

    }
    
    return count;
}

int ADC1_ResultGetFromChannel(int channel) {
    
    int conversion;                     // Return value

    AD1CHS = ADC_CHANNEL_MAP[channel];      // Sets ADC Channel
    
    AD1CON1bits.ADON = 1;                   // Enables ADC (turned on/off to reset buffer)
    
    wait_for(100);
    
    ADC1_ManualSampleForSec(500);             // Sample ADC for 1s [*needs refining*]
    
<<<<<<< HEAD:UNITE_Main.X/adc1.c
    wait_for(100);                          // Wait for conversion to finish (5s) [*needs refining*]
=======
    wait_ms(10);                            // Wait for conversion to finish (5s) [*needs refining*]
>>>>>>> master:BackupofMCC/adc1.c
        
    conversion = ADC1BUF0;                  // Get conversion value from buffer
    
    AD1CON1bits.ADON = 0;                   // Disable ADC to reset buffer
    
    return conversion;
        
}

void ADC1_ManualSampleForSec(unsigned long sec) {
    
    _SAMP = 1;                              // Begin Sampling
    wait_for(sec);                            // Wait to gather sample data
    _SAMP = 0;                              // Stop sampling
}
