/* 
 * File:   adc1.h
 * Author: zacksnyder
 *
 * Created on January 26, 2017, 3:29 PM
 */

#ifndef ADC1_H
#define	ADC1_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ADC1_H */

// ADC_CHANNEL stores hex values for ADC1CHS (sets ADC channel)

typedef enum 
{
    ADC_CHANNEL_AN2 =  0x2,
    ADC_CHANNEL_AN3 =  0x3,
    ADC_CHANNEL_AN4 =  0x4,
    ADC_CHANNEL_AN5 =  0x5,
    ADC_CHANNEL_AN6 =  0x6,
    ADC_CHANNEL_AN7 =  0x7,
    ADC_CHANNEL_AN8 =  0x8,
    ADC_CHANNEL_AN9 =  0x9,
    ADC_CHANNEL_AN10 =  0xA,
    ADC_CHANNEL_AN11 =  0xB,
    ADC_CHANNEL_AN12 =  0xC,
    ADC_CHANNEL_AN13 =  0xD,
    ADC_CHANNEL_AN14 =  0xE,
    ADC_CHANNEL_AN15 =  0xF,
    ADC_CHANNEL_VBG_2 =  0x10,
    ADC_CHANNEL_VBG =  0x11,
    ADC_MAX_CHANNEL_COUNT = 16
} ADC_CHANNEL;




void InitializeADC1(void);

uint16_t ADC1_ResultGet(uint16_t *buffer);
uint8_t ADC1_ResultGetFromChannel(int);
void ADC1_ManualSampleForSec(unsigned long);