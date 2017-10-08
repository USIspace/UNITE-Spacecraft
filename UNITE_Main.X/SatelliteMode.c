#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"
#include "adc1.h"
#include "mcc_generated_files/uart3.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr5.h"

/*******************
  Global Variables
 *******************/

// Stores the current operational mode of UNITE
// Used as a switch for the satellite to change modes
UNITEMode currentMode = safe;
bool shouldChangeMode = false;
unsigned long lastAltitude = 400; // 400 km
unsigned long totalTime = 0; // Keeps track of overall mission clock
double timeInMin = 0.0;      // Time in Min since 00:00

int currentLogWait = 0;

/*******************************
  Satellite Mode Configurations
 *******************************/

SatelliteMode StartupMode = {
    400,    // Altitude to begin sampling in this mode
    390,    // Altitude to end sampling and switch to new mode
    5       // Days until needs to switch mode
};

SatelliteMode InterimMode = {
    390,  
    300, 
    380, 
};

SatelliteMode ScienceMode = {
    300,
    225,
    420
};

SatelliteMode ReEntryMode = {
    225,
    0,
    500
};

SatelliteMode SafeMode = {
    0,
    0,
    0
};

/******************************
 Satellite Mode Initializations
 ******************************/

void SatelliteProperties_Initialize() {
    // Set Current Mode to interim on start
    currentMode = UpdateMode();
    TMR1_INTERRUPT_TICKER_FACTOR = 1;   // LANGMUIR PROBE CLOCK
    TMR5_INTERRUPT_TICKER_FACTOR = 10;  // MAIN LOOP CLOCK
    
    // Initialize Debug light to on
   _LATE1 = LED_ON;
   _LATE2 = LED_ON;
   
   _LATE3 = LED_ON;
   _LATE4 = LED_ON;
   
   //Slave select 1
   _LATG7 = 1;   
   
   //Set Satellite Time
//   SetDuplexPower(1);
   SetTotalTime();
//   SetDuplexPower(0);
}

 
void Satellite_Initialize() {

    // Initialize Analog/Digital Converter
    InitializeADC1();
    
    // Initialize main variables
    SatelliteProperties_Initialize();
    
    // Initialize Main Mode Timer
    TMR5_Initialize();
    TMR1_Initialize();
    
    // Start Main Timer
    TMR5_Start();
    TMR1_Start();
}

/********************
 Mode Switch Handling
 ********************/

// Description: Handler for switching satellite modes
// time -> total mission duration in days
// altitude -> last known altitude of the satellite in meters
// Return: Boolean for whether a mode switch should occur
bool ShouldUpdateMode(unsigned long time, unsigned long altitude) {

    switch (currentMode) {
        case safe: return true;
        case startup: 
            if (time > StartupMode.stopTime) return true;
            else if ((altitude) <= StartupMode.endAltitudeInKm) return true;
        case interim:
            if (time > InterimMode.stopTime) return true;
            else if ((altitude) <= InterimMode.endAltitudeInKm) return true;
        case science:
            if (time > ScienceMode.stopTime) return true;
            else if ((altitude) <= ScienceMode.endAltitudeInKm) return true;
        case reentry: break;
            if (time > ReEntryMode.stopTime) return true;
            else if ((altitude) <= ReEntryMode.endAltitudeInKm) return true;
    }
    return false;
}

// Description: Getter for the correct operational mode

UNITEMode UpdateMode() {

    if (ShouldUpdateMode(totalTime / 3600, lastAltitude)) {
        
        /*
        int i;
        for (i = 0; i < 8; i++) {
            UART3_Write(255); 
            UART3_Write(0);
        }*/
        
        switch (currentMode) {
            case startup: 
                
                _LATE2 = LED_ON;
                _LATE3 = LED_OFF;
                _LATE4 = LED_OFF;
                
                return interim;
            case interim:
                
                _LATE2 = LED_OFF;
                _LATE3 = LED_ON;
                _LATE4 = LED_OFF;

                return science;
            case science:
                
                _LATE2 = LED_OFF;
                _LATE3 = LED_OFF;
                _LATE4 = LED_ON;

                return reentry;
            case reentry:
                
                return currentMode; // Never leave reentry mode

            case safe:

                _LATE2 = LED_OFF;
                _LATE3 = LED_OFF;
                _LATE4 = LED_OFF;
                
                return startup;
        }
    }
    
    return currentMode;
}

/*******************
  Controller Method
 *******************/

void MainLoop() {
    
    // Langmuir Probe
    TrySampleLangmuirProbe();
    
    // Magnetometer
    TrySampleMagnetometer();
    
    // Temperature Sensors
    TrySampleTemperature();
    
    // GPS
    TrySampleGPS();
    
    // Housekeeping
    TrySampleHousekeeping();
    
    // Send Power Switch Packet
    TogglePowerSwitches();
        
    // Transmission
    TransmitQueue();
 
    // Update Time    
    totalTime += TMR5_INTERRUPT_TICKER_FACTOR;
    timeInMin += (double)TMR5_INTERRUPT_TICKER_FACTOR / 60.0;
    if (timeInMin > 1440) timeInMin -= 1440;
    
    // Update SatelliteMode
    currentMode = UpdateMode();
        
    // Commanding
    HandleCommand();
    
    // Log
    if (IS_DIAG) {
        if (++currentLogWait >= 1) { 
            LogState();
            currentLogWait = 0;
            
            SetAltitude(0.0);
        }
    }
    
//    TestDACSPI();
    
    
}

void SetTime(double formattedTime) {
    
    // 9h 56min 24sec . 00
    int hours = (formattedTime / 10000) * 60;   // hours = 9h * 60 min/h
    formattedTime = (int)formattedTime % 10000;                     // formattedTime = 56min 24sec
    int min = formattedTime / 100;              // min = 56min
    formattedTime = (int)formattedTime % 100;                       // formattedTime = 24sec
    double sec = formattedTime / 60;            // sec = 0.40 min
    timeInMin = hours + min + sec;
}

//void SetTime(uint8_t *time, int arrayLength) {
//
//    int hours = 0;
//    int mins = 0;
//    
//    // Convert string to int    
//    int i;
//    for (i = 0; i < arrayLength; i++) {
//        switch (i) {
//            case 0: 
//                hours += time[i] & 0x0F;
//                hours += ((time[i] & 0xF0) >> 4) * 10;
//                break;
//            case 1: 
//                mins += time[i] & 0x0F;
//                mins += ((time[i] & 0xF0) >> 4) * 10;
//                break;
//            default: break;
//        }
//    }
//    
//    timeInMin = ((hours * 60) + mins);
//}

void SetAltitude(double alt) {
    
     lastAltitude -= 1;
//    lastAltitude = (int)alt;
}

/*
void SetAltitude(uint8_t *alt, int arrayLength) {
    
    unsigned long convertedAltitude = 0;

    char *altString = (char *)alt;
    char *decimal = strrchr(altString, '.');
    int decimals = (int)&alt - (int)&decimal;
    
    // Convert string to int
    int altIntLength = (arrayLength- decimals);
    
    int i;
    for (i = 0; i < altIntLength; i++) {
        int exp = ((altIntLength) - (i * 2));
        convertedAltitude += (alt[i] & 0x0F) * Pow(10, exp);
        convertedAltitude += ((alt[i] & 0xF0) >> 4) * 10 * Pow(10,exp);
    }
    
    if (convertedAltitude > 0) lastAltitude = convertedAltitude;
}*/

time_t logCount = 0;

// Logging Method for Diagnostic Mode
void LogState() {
    
    char log[1000] = "UNITE Log #";
    char newLine[] = "\n\n";
    char endLine[] = "EOF\n";
    
    //Log Count
    char count[10];
    sprintf(count, "%u", (unsigned int)logCount++);
    strcat(log, count);
    
    strcat(log, newLine);
    
    //Current Mode

    switch (currentMode) {
        case startup: strcat(log, "Current Mode: startup"); break;
        case interim: strcat(log, "Current Mode: interim"); break;
        case science: strcat(log, "Current Mode: science"); break;
        case reentry: strcat(log, "Current Mode: reentry"); break;
        default: break;
    }
    
    strcat(log, newLine);
    
    //Altitude
    char altitude[20];
    sprintf(altitude, "Altitude: %u km", (unsigned int)lastAltitude);
    strcat(log, altitude);
    
    strcat(log, newLine);
    
    //Time 
    char timeString[50];
    sprintf(timeString, "Total runtime: %u h %d min\nTime of day: %d:%d UTC", (unsigned int)(totalTime / 3600), (int)(totalTime / 60), (int)(timeInMin / 60),(int)timeInMin % 60);
    strcat(log, timeString);
    
    strcat(log, newLine);
    
    //Housekeeping
    char diagData[500];
    sprintf(diagData,
            "Battery 1 Charge: %u \nBattery 2 Charge: %u \nBattery 1 Voltage: %u \nBattery 2 Voltage: %u \nBattery 1 Current: %u \nBattery 2 Current: %u \nBuss+ Voltage: %u \nSolar Panel 1 Voltage: %u \nSolar Panel 2 Voltage: %u \nSolar Panel 3 Voltage: %u \nSolar Panel 4 Voltage: %u \nSimplex Temp: %u \nDuplex Temp: %u \nEPS Temp: %u \n\nLP Temp: %u \nLP Cal: %u, %u, %u, %u \nMagnetometer x: %u, y: %u, z: %u \nTemperature: %u, %u, %u, %u, %u, %u, %u, %u",
            (unsigned int)b1Charge,
            (unsigned int)b2Charge,
            (unsigned int)b1Voltage,
            (unsigned int)b2Voltage,
            (unsigned int)b1Current,
            (unsigned int)b2Current,
            (unsigned int)bussPlusVoltage,
            (unsigned int)solar1Voltage,
            (unsigned int)solar2Voltage,
            (unsigned int)solar3Voltage,
            (unsigned int)solar4Voltage,
            (unsigned int)simplexTemp,
            (unsigned int)duplexTemp,
            (unsigned int)epsTemp,
            (unsigned int)langmuirProbeDiagData[0],
            (unsigned int)langmuirProbeDiagData[1],
            (unsigned int)langmuirProbeDiagData[2],
            (unsigned int)langmuirProbeDiagData[3],
            (unsigned int)langmuirProbeDiagData[4],
            (unsigned int)magnetometerDiagData[0],
            (unsigned int)magnetometerDiagData[1],
            (unsigned int)magnetometerDiagData[2],
            (unsigned int)temperatureDiagData[0],
            (unsigned int)temperatureDiagData[1],
            (unsigned int)temperatureDiagData[2],
            (unsigned int)temperatureDiagData[3],
            (unsigned int)temperatureDiagData[4],
            (unsigned int)temperatureDiagData[5],
            (unsigned int)temperatureDiagData[6],
            (unsigned int)temperatureDiagData[7]);
    
    strcat(log, diagData);
    
    strcat(log, newLine);
    strcat(log, endLine);
    
    uint16_t length = strlen(log);
    int i;
    for (i = 0; i < length; i++) {
        Send((uint8_t)log[i], DiagUnit);
    }
}