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
#include "UART.h"

int SerialSend(TxByte) 
{
    
    U1TXREG = TxByte;
    return 0;
}

/* Serial Available allows for the user to check if there is data in the register 
 * by seeing if the return value from the function is zero, there is an error if
 * a 1 is returned, calling the fuction SerialRead will allow to read the UART
 * register/buffer
 
 */
int SerialAvailable()
{
    
    
    /* Check for receive errors */
    if(U1STAbits.FERR == 1)
    {  
        return 0;
    }


    if(U1STAbits.OERR == 1)
    {
        return 0;
    } 
    //
    
    
    if(U1STAbits.URXDA == 1){
        return 1;
    }
       
    return 9; //This needs to be changed but is fine for right now - colin
}
int SerialRead()
{
    int RxByte = 0;
    RxByte = U1RXREG;
    return RxByte; 
}