/**
  @Generated MPLAB(c) Code Configurator Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    mcc.c

  @Summary:
    This is the mcc.c file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - pic24-dspic-pic32mm : v1.25
        Device            :  PIC24FJ256GA106
        Driver Version    :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC16 1.26
        MPLAB             :  MPLAB X 3.45
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

// Configuration bits: selected in the GUI

// CONFIG3
#pragma config WPFP = WPFP511    // Write Protection Flash Page Segment Boundary->Highest Page (same as page 170)
#pragma config WPDIS = WPDIS    // Segment Write Protection Disable bit->Segmented code protection disabled
#pragma config WPCFG = WPCFGDIS    // Configuration Word Code Page Protection Select bit->Last page(at the top of program memory) and Flash configuration words are not protected
#pragma config WPEND = WPENDMEM    // Segment Write Protection End Page Select bit->Write Protect from WPFP to the last page of memory

// CONFIG2
#pragma config POSCMOD = NONE    // Primary Oscillator Select->Primary oscillator disabled
#pragma config IOL1WAY = ON    // IOLOCK One-Way Set Enable bit->Write RP Registers Once
#pragma config OSCIOFNC = OFF    // Primary Oscillator Output Function->OSCO functions as CLKO (FOSC/2)
#pragma config FCKSM = CSDCMD    // Clock Switching and Monitor->Both Clock Switching and Fail-safe Clock Monitor are disabled
#pragma config FNOSC = FRCPLL    // Oscillator Select->Fast RC oscillator with Postscaler and PLL module (FRCPLL)
#pragma config IESO = ON    // Internal External Switch Over Mode->IESO mode (Two-speed start-up) enabled

// CONFIG1
#pragma config WDTPS = PS32768    // Watchdog Timer Postscaler->1:32768
#pragma config FWPSA = PR128    // WDT Prescaler->Prescaler ratio of 1:128
#pragma config WINDIS = OFF    // Watchdog Timer Window->Standard Watchdog Timer is enabled,(Windowed-mode is disabled)
#pragma config FWDTEN = OFF    // Watchdog Timer Enable->Watchdog Timer is disabled
#pragma config ICS = PGx1    // Comm Channel Select->Emulator functions are shared with PGEC1/PGED1
//#pragma config COE = OFF    // Set Clip On Emulation Mode->Disabled
//#pragma config BKBUG = OFF    // Background Debug->Device resets into Operational mode
#pragma config GWRP = OFF    // General Code Segment Write Protect->Writes to program memory are allowed
#pragma config GCP = OFF    // General Code Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    // JTAG Port Enable->JTAG port is disabled

#include "mcc.h"

void SYSTEM_Initialize(void)
{
    PIN_MANAGER_Initialize();
    OSCILLATOR_Initialize();
    INTERRUPT_Initialize();
    UART1_Initialize();
    UART3_Initialize();
    TMR4_Initialize();
    TMR2_Initialize();
    UART2_Initialize();
    UART4_Initialize();
    //ADC1_Initialize(); //If using custom class, comment this line
    TMR3_Initialize();
    TMR5_Initialize();
    TMR1_Initialize();
}

void OSCILLATOR_Initialize(void)
{
    // NOSC FRCPLL; SOSCEN disabled; OSWEN Switch is Complete; 
    __builtin_write_OSCCONL((uint8_t) (0x0100 & 0x00FF));
    // RCDIV FRC/1; DOZE 1:1; DOZEN disabled; ROI disabled; 
        
    CLKDIV = 0x0000;
    // TUN Center frequency; 
    OSCTUN = 0x0000;
    // WDTO disabled; TRAPR disabled; SWDTEN disabled; EXTR disabled; POR disabled; SLEEP disabled; BOR disabled; IDLE disabled; IOPUWR disabled; VREGS disabled; CM disabled; SWR disabled; 
    RCON = 0x0000;
    
    _RCDIV=0;
    _DOZE=0;
    _DOZEN=0;
}

/**
 End of File
*/