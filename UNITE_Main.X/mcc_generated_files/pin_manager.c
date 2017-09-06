/**
  System Interrupts Generated Driver File 

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.c

  @Summary:
    This is the generated manager file for the MPLAB(c) Code Configurator device.  This manager
    configures the pins direction, initial state, analog setting.
    The peripheral pin select, PPS, configuration is also handled by this manager.

  @Description:
    This source file provides implementations for MPLAB(c) Code Configurator interrupts.
    Generation Information : 
        Product Revision  :  MPLAB(c) Code Configurator - 4.15
        Device            :  PIC24FJ256GA106
        Version           :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC16 1.26
        MPLAB             :  MPLAB X 3.45

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

*/


/**
    Section: Includes
*/
#include <xc.h>
#include "pin_manager.h"

/**
    void PIN_MANAGER_Initialize(void)
*/
void PIN_MANAGER_Initialize(void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;
    LATE = 0x0000;
    LATF = 0x0000;
    LATG = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISB = 0xFFFF;
    TRISC = 0x7000;
    TRISD = 0x0EE5;
    TRISE = 0x0001;  // 0x0021 previously
    TRISF = 0x006B;
    TRISG = 0x020C;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPD1 = 0x0000;
    CNPD2 = 0x0000;
    CNPD3 = 0x0000;
    CNPD4 = 0x0000;
    CNPD5 = 0x0000;
    CNPD6 = 0x0000;
    CNPU1 = 0x0000;
    CNPU2 = 0x0000;
    CNPU3 = 0x0000;
    CNPU4 = 0x0000;
    CNPU5 = 0x0000;
    CNPU6 = 0x0000;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCB = 0x0000;
    ODCC = 0x0000;
    ODCD = 0x0000;
    ODCE = 0x0000;
    ODCF = 0x0000;
    ODCG = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    AD1PCFGH = 0x0000;
    AD1PCFGL = 0x0003;

    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS

    RPOR12bits.RP25R = 0x000C;    //RD4->SPI2:SS2OUT;
//    RPOR9bits.RP19R = 0x0009;    //RG8->SPI1:SS1OUT;
    RPOR12bits.RP24R = 0x000B;    //RD1->SPI2:SCK2OUT;
    RPOR11bits.RP22R = 0x000A;    //RD3->SPI2:SDO2;
    RPOR15bits.RP30R = 0x0005;    //RF2->UART2:U2TX;
    RPOR10bits.RP21R = 0x0008;    //RG6->SPI1:SCK1OUT;
//    RPOR13bits.RP26R = 0x0008;    //RG7->SPI1:SCK1OUT;
    RPINR19bits.U2RXR = 0x002D;   //RF6->UART2:U2RX;
    RPOR5bits.RP10R = 0x0003;     //RF4->UART1:U1TX;
    RPINR22bits.SDI2R = 0x0017;   //RD2->SPI2:SDI2;
    RPINR18bits.U1RXR = 0x0011;   //RF5->UART1:U1RX;
    RPOR9bits.RP19R = 0x0007;     //RG8->SPI1:SDO1;
    RPINR17bits.U3RXR = 0x0004;   //RD9->UART3:U3RX;
//    RPINR20bits.SDI1R = 0x001A;   //RG7->SPI1:SDI1;
    RPOR1bits.RP2R = 0x001C;      //RD8->UART3:U3TX;
    RPINR27bits.U4RXR = 0x000C;   //RD11->UART4:U4RX
    RPOR5bits.RP11R = 0x001E;     //RD0->UART4:U4TX

    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS

}

