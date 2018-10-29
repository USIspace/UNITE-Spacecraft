
/**
  RTCC Generated Driver API Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    rtcc.c

  @Summary:
    This is the generated header file for the RTCC driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides APIs for driver for RTCC.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - pic24-dspic-pic32mm : 1.65
        Device            :  PIC24FJ256GA106
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.35
        MPLAB 	          :  MPLAB X v4.20
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


/**
 Section: Included Files
*/

#include <xc.h>
#include <time.h>
#include "rtcc.h"

/**
* Section: Function Prototype
*/
static bool rtccTimeInitialized;
static bool RTCCTimeInitialized(void);
static uint8_t ConvertHexToBCD(uint8_t hexvalue);
static uint8_t ConvertBCDToHex(uint8_t bcdvalue);

/**
* Section: Driver Interface Function Definitions
*/

void RTCC_Initialize(void)
{

   // Set the RTCWREN bit
   __builtin_write_RTCWEN();

   RCFGCALbits.RTCEN = 0;
   
   if(!RTCCTimeInitialized())
   {
       // set RTCC time 2018-09-15 20-06-55
       RCFGCALbits.RTCPTR = 3;        // start the sequence
       RTCVAL = 0x18;    // YEAR
       RTCVAL = 0x915;    // MONTH-1/DAY-1
       RTCVAL = 0x623;    // WEEKDAY/HOURS
       RTCVAL = 0x1117;    // MINUTES/SECONDS
   }


   // RTSECSEL Alarm Pulse; 
   PADCFG1 = 0x0000;
           
   // Enable RTCC, clear RTCWREN
   RCFGCALbits.RTCEN = 1;
   RCFGCALbits.RTCWREN = 0;

}


/**
 This function implements RTCC_TimeReset.This function is used to
 used by application to reset the RTCC value and reinitialize RTCC value.
*/
void RTCC_TimeReset(bool reset)
{
    rtccTimeInitialized = reset;
}

static bool RTCCTimeInitialized(void)
{
    return(rtccTimeInitialized);
}

/**
 This function implements RTCC_TimeGet. It access the 
 registers of  RTCC and writes to them the values provided 
 in the function argument currentTime
*/

bool RTCC_TimeGet(struct tm *currentTime)
{
    uint16_t register_value;
    if(RCFGCALbits.RTCSYNC){
        return false;
    }

    RCFGCALbits.RTCPTR = 3;
    register_value = RTCVAL;
    currentTime->tm_year = ConvertBCDToHex(register_value & 0x00FF);
    register_value = RTCVAL;
    currentTime->tm_mon = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_mday = ConvertBCDToHex(register_value & 0x00FF);
    register_value = RTCVAL;
    currentTime->tm_wday = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_hour = ConvertBCDToHex(register_value & 0x00FF);
    register_value = RTCVAL;
    currentTime->tm_min = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_sec = ConvertBCDToHex(register_value & 0x00FF);

    return true;
}

/**
 * This function sets the RTCC value and takes the input time in decimal format
*/

void RTCC_TimeSet(struct tm *initialTime)
{
   // Set the RTCWREN bit
   __builtin_write_RTCWEN();

   RCFGCALbits.RTCEN = 0;
   

   // set RTCC initial time
   RCFGCALbits.RTCPTR = 3;                               // start the sequence
   RTCVAL =  ConvertHexToBCD(initialTime->tm_year);                        // YEAR
   RTCVAL = (ConvertHexToBCD(initialTime->tm_mon) << 8) | ConvertHexToBCD(initialTime->tm_mday);  // MONTH-1/DAY-1
   RTCVAL = (ConvertHexToBCD(initialTime->tm_wday) << 8) | ConvertHexToBCD(initialTime->tm_hour); // WEEKDAY/HOURS
   RTCVAL = (ConvertHexToBCD(initialTime->tm_min) << 8) | ConvertHexToBCD(initialTime->tm_sec);   // MINUTES/SECONDS
             
   // Enable RTCC, clear RTCWREN         
   RCFGCALbits.RTCEN = 1;  
   RCFGCALbits.RTCWREN = 0;
   

}

time_t RTCC_TimeElapsedInSec(struct tm *previousTime) 
{
    
    struct tm currentTime; 
    RTCC_TimeGet(&currentTime);
    
    time_t delta = (currentTime.tm_year - previousTime->tm_year) * 24 * 3600 + (currentTime.tm_hour - previousTime->tm_hour) * 3600 + (currentTime.tm_min - previousTime->tm_min) * 60 + (currentTime.tm_sec - previousTime->tm_sec);
    
    return delta;
}

/**
 This function reads the RTCC time and returns the date and time in BCD format
  */
bool RTCC_BCDTimeGet(bcdTime_t *currentTime)
{
    uint16_t register_value;
    if(RCFGCALbits.RTCSYNC){
        return false;
    }

    RCFGCALbits.RTCPTR = 3;
    register_value = RTCVAL;
    currentTime->tm_year = register_value;
    register_value = RTCVAL;
    currentTime->tm_mon = (register_value & 0xFF00) >> 8;
    currentTime->tm_mday = register_value & 0x00FF;
    register_value = RTCVAL;
    currentTime->tm_wday = (register_value & 0xFF00) >> 8;
    currentTime->tm_hour = register_value & 0x00FF;
    register_value = RTCVAL;
    currentTime->tm_min = (register_value & 0xFF00) >> 8;
    currentTime->tm_sec = register_value & 0x00FF;

    return true;
}
/**
 This function takes the input date and time in BCD format and sets the RTCC
 */
void RTCC_BCDTimeSet(bcdTime_t *initialTime)
{
   // Set the RTCWREN bit
   __builtin_write_RTCWEN();

   RCFGCALbits.RTCEN = 0;


   // set RTCC initial time
   RCFGCALbits.RTCPTR = 3;                               // start the sequence
   RTCVAL = initialTime->tm_year;                        // YEAR
   RTCVAL = (initialTime->tm_mon << 8) | initialTime->tm_mday;  // MONTH-1/DAY-1
   RTCVAL = (initialTime->tm_wday << 8) | initialTime->tm_hour; // WEEKDAY/HOURS
   RTCVAL = (initialTime->tm_min << 8) | initialTime->tm_sec;   // MINUTES/SECONDS

   // Enable RTCC, clear RTCWREN
   RCFGCALbits.RTCEN = 1;
   RCFGCALbits.RTCWREN = 0;

}

static uint8_t ConvertHexToBCD(uint8_t hexvalue)
{
    uint8_t bcdvalue;
    bcdvalue = (hexvalue / 10) << 4;
    bcdvalue = bcdvalue | (hexvalue % 10);
    return (bcdvalue);
}

static uint8_t ConvertBCDToHex(uint8_t bcdvalue)
{
    uint8_t hexvalue;
    hexvalue = (((bcdvalue & 0xF0) >> 4)* 10) + (bcdvalue & 0x0F);
    return hexvalue;
}


/* Function:
    bool RTCC_Task(void)

  Summary:
    Status function which returns the RTCC interrupt flag status

  Description:
    This is the status function for the RTCC peripheral. 
*/
bool RTCC_Task(void)
{
    bool status;
    status = IFS3bits.RTCIF;
    if( IFS3bits.RTCIF)
    {
       IFS3bits.RTCIF = false;
    }
    return status;
}


/**
 End of File
*/
