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
#include "UART.h"

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
    ConfigureOscillator();

    /* Initialize IO ports and peripherals */
    InitApp();
    
    /* Set pins as input or output  */
    AD1PCFGLbits.PCFG14 = 0; // initializes pin as ADC input
    AD1PCFGLbits.PCFG15 = 0; // initializes pin as ADC input 
    /*Step 1 from datasheet*/

    U1MODEbits.PDSEL = 0;   // 8-bit data, no parity
    U1MODEbits.STSEL = 0;   // One stop bit
    U1BRG = 25;             // sets baud rate (9600) for 8Mhz Fosc (25))
    
    /*Step 2 from datasheet*/
    U1MODEbits.UARTEN = 1;  // enables UART
    
     /*Step 3 from datasheet*/
    U1STAbits.UTXEN = 1;    // enable transmit

    /*More UART Setup*/ 
    
    U1STAbits.URXISEL0 = 0; // receive interrupt mode
    U1STAbits.URXISEL1 = 0; // receive interrupt mode
    U1MODEbits.USIDL = 0;   // disables "stop in idle mode"
    U1MODEbits.IREN = 0;    // disables IrDA
    U1MODEbits.RTSMD = 1;   // Simplex Mode
    U1MODEbits.UEN0 = 0;    // UART1 enable bits
    U1MODEbits.UEN1 = 0;    // UART1 enable bits
    U1MODEbits.LPBACK = 0;  // disables loop back
    U1MODEbits.ABAUD = 0;   // disables auto baud rate
    U1MODEbits.RXINV = 0;   // UxRX idle state is 1
    U1MODEbits.BRGH = 0;    // Standard mode (not high baud rate)
    U1STAbits.UTXISEL0 = 1; // Transmission interrupt mode
    U1STAbits.UTXISEL1 = 1; // Transmission interrupt mode

       
 
    int count = 5; 
    SerialSend(count);


    while(1)
    { 
        
        if (SerialAvailable() == 1){ //This checks to see if the data is good and available
        count = SerialRead(); //This will receive data
        SerialSend(count); //This will send data
        } else {
            continue;
        }
        
        
        
    }        
}