/* 
 * File:   UART.h
 * Author: cdrunnion
 *
 * Created on January 15, 2017, 4:04 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
    /* Initialize UART registers*/
    
    


int SerialSend( int RxByte);
int SerialRecieve();
int SerialAvailable();
