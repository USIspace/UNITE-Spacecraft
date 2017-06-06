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

#include <stdint.h>          /* For uint32_t definition */
#include <stdbool.h>         /* For true/false definition */

#include "system.h"          /* variables/params used by system.c */
#include "time.h"

/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.c                                           */
/*                                                                            */
/******************************************************************************/

/* Refer to the device Family Reference Manual Oscillator section for
information about available oscillator configurations.  Typically
this would involve configuring the oscillator tuning register or clock
switching using the compiler's __builtin_write_OSCCON functions.
Refer to the C Compiler for PIC24 MCUs and dsPIC DSCs User Guide in the
compiler installation directory /doc folder for documentation on the
__builtin functions. */

/* TODO Add clock switching code if appropriate.  An example stub is below.   */
void ConfigureOscillator(void)
{

    
    __builtin_write_OSCCONH(0b010);
    __builtin_write_OSCCONL(OSCCON | 0x01);
    while (OSCCONbits.COSC != 0b010 || OSCCONbits.OSWEN == 1);
    
#if 0

    /* Disable Watch Dog Timer */
    RCONbits.SWDTEN = 0;

    __builtin_write_OSCCONH(0x02); // Initiate Clock Switch to FRC with PLL (NOSC=0b001)
    
    if (_OSWEN == 0) { // Check clock switch is not in progress
        __builtin_write_OSCCONL(0x01); // Start clock switching
        while (OSCCONbits.COSC != 0b010); // Wait for Clock switch to occur
    }//end if
    
    

    /* Wait for Clock switch to occur */
    /* Wait for PLL to lock, if PLL is used */
    /*while(OSCCONbits.LOCK != 1); */

#endif
        
}

/*
void wait_sec(unsigned long time) {
    unsigned long counter;
    for (counter = 0; counter < time; counter++) {
        wait_ms(1000);
    }
}*/

/*
void wait_sec(unsigned long seconds)
{
    //unsigned long timel = 0;
        
    time_t *zero;
    unsigned long currentTime = time(NULL);
    time_t timeToStop = currentTime + seconds;
    
    while (time(NULL) < timeToStop){
        unsigned long dummy = time(NULL);
        wait_for(10);
    }
    //timel = time * 1500;
    //for( ; timel; timel--);// no initial condition, while time is >0, decrement time each loop
}*/

void wait_for(unsigned long something) {
    
    unsigned long wait = something * 1000;
    uint16_t i;
    for (i = 0; i < wait; i++);
}

uint16_t Pow(int base, int exponent) {
  uint16_t returnValue = 1;
  int i;
  for (i = 0; i < exponent; i++) {
      returnValue = returnValue * base;
  }
  
  return returnValue;
}

void ClearQueue(uint8_t *buffer, int size, int startIndex) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i + startIndex] = 0;
    }
}

void Clear(int *buffer, int size, int startIndex) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i + startIndex] = 0;
    }
}

void Copy(int *source, uint8_t *destination, int sourceStart, int destStart, int numberOfItems) {
    
    int i;
    for (i = sourceStart; i < numberOfItems; i++) {
        destination[i + destStart] = source[i];
    }
}
