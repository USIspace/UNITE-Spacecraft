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
#include "mcc_generated_files/rtcc.h"

/*******************
  Global Variables
 *******************/

// Stores the current operational mode of UNITE
// Used as a switch for the satellite to change modes
UNITEMode currentMode = safe;
bool shouldChangeMode = false;
double lastAltitude = 400.0; // 400 km
uint16_t duplexEpoch = 0;
time_t previousTime;
time_t totalTime = 0; // Keeps track of overall mission clock in seconds
double timeInMin = 0.0;      // Time in Min since 00:00

int currentLogWait = 0;

/*******************************
  Satellite Mode Configurations
 *******************************/

SatelliteMode FirstWeekMode = {
    400,    // Altitude to begin sampling in this mode
    390,    // Altitude to end sampling and switch to new mode
    5,       // Days until needs to switch mode
    "First Week"
};

SatelliteMode InterimMode = {
    390,  
    325, 
    300, 
    "Interim"
};

SatelliteMode StabilizeMode = {
    325,
    300,
    380,
    "Stabilize"
};

SatelliteMode ScienceMode = {
    300,
    225,
    400,
    "Science"
};

SatelliteMode ReEntryMode = {
    225,
    0,
    1000000,
    "Reentry"
};

SatelliteMode SafeMode = {
    0,
    0,
    0,
    "Safe"
};

/******************************
 Satellite Mode Initializations
 ******************************/

void SatelliteProperties_Initialize() {
    // Set Current Mode to interim on start
    currentMode = UpdateMode();
    TMR1_INTERRUPT_TICKER_FACTOR = 1;   // LANGMUIR PROBE CLOCK
    TMR5_INTERRUPT_TICKER_FACTOR = MAIN_LOOP_TIMER_INTERVAL;  // MAIN LOOP CLOCK
    
    // Initialize Debug light to on
   _LATE1 = LED_ON;
   _LATE2 = LED_ON;
   
   _LATE3 = LED_ON;
   _LATE4 = LED_ON;
   
   //Slave select 1
   _LATG7 = 1;   
   
   //Set Satellite Time
   previousTime = time(NULL);
   
   
   SetDuplexPower(1);
   SetTotalTime();
   SetDuplexPower(0);
   
   // Wait times
//   currentLangmuirProbeWait = totalTime / 60;
//   currentMagnetometerWait = totalTime / 60;
//   currentTemperatureWait = totalTime / 60;
//   currentGPSWait = totalTime / 60;
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

    if ((gpsLockAttempts > GPS_LOCK_FAILURE && waitingFilesCount >= MAX_DUP_FILES_WAITING) && !IS_DIAG) return true;
    
    switch (currentMode) {
        case safe: return true;
        case firstWeek: 
            if (time / 24 > FirstWeekMode.stopTime) return true;
            else if ((altitude) <= FirstWeekMode.endAltitudeInKm) return true;
        case interim:
            if (time / 24 > InterimMode.stopTime) return true;
            else if ((altitude) <= InterimMode.endAltitudeInKm) return true;
        case stabilize: 
            if (time / 24 > StabilizeMode.stopTime) return true;
            else if ((altitude) <= StabilizeMode.endAltitudeInKm) return true;
        case science:
            if (time / 24 > ScienceMode.stopTime) return true;
            else if ((altitude) <= ScienceMode.endAltitudeInKm) return true;
        case reentry: break;
            if (time / 24> ReEntryMode.stopTime) return true;
            else if ((altitude) <= ReEntryMode.endAltitudeInKm) return true;
        case fallback: //break;
            if (gpsLockAttempts < GPS_LOCK_FAILURE) return true;
            if (waitingFilesCount < MAX_DUP_FILES_WAITING) return true;
            
            
    }
    return false;
}

// Description: Getter for the correct operational mode

UNITEMode UpdateMode() {

    if (ShouldUpdateMode(totalTime / 3600, (int)lastAltitude)) {
        
        if ((gpsLockAttempts > GPS_LOCK_FAILURE && waitingFilesCount >= MAX_DUP_FILES_WAITING) && !IS_DIAG) {
            
//            _LATE2 = LED_ON;
//            _LATE3 = LED_OFF;
//            _LATE4 = LED_ON;
            
            char fallbackName[] = "Fallback";
            
            PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)fallbackName, sizeof(fallbackName));
            
            return fallback;
        }
        
        switch (currentMode) {
            case firstWeek: 
                
//                _LATE2 = LED_ON;
//                _LATE3 = LED_OFF;
//                _LATE4 = LED_OFF;
                
                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)InterimMode.name, strlen(InterimMode.name));
                
                return interim;
            case interim:
                
//                _LATE2 = LED_ON;
//                _LATE3 = LED_ON;
//                _LATE4 = LED_OFF;
                
                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)StabilizeMode.name, strlen(StabilizeMode.name));

                return stabilize;
            case stabilize:
                
//                _LATE2 = LED_OFF;
//                _LATE3 = LED_ON;
//                _LATE4 = LED_OFF;
                
                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)ScienceMode.name, strlen(ScienceMode.name));
                
                return science;
            case science:
                
//                _LATE2 = LED_OFF;
//                _LATE3 = LED_OFF;
//                _LATE4 = LED_ON;

                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)ReEntryMode.name, strlen(ReEntryMode.name));
                
                return reentry;
            case reentry:
                
                return currentMode; // Never leave reentry mode

            case fallback:
                
                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)InterimMode.name, strlen(InterimMode.name));
                
                return interim;
                
            case safe:

//                _LATE2 = LED_OFF;
//                _LATE3 = LED_OFF;
//                _LATE4 = LED_OFF;
                
                PackageData(CDHSubSys, (int)timeInMin, (uint8_t *)FirstWeekMode.name, strlen(FirstWeekMode.name));
                
                return firstWeek;
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
    
    // Transmission
    TransmitQueue();
    
    // Send Power Switch Packet
    TogglePowerSwitches();
 
    // Update Time 
    
    time_t timeDelta = (time(NULL) - previousTime) / 12670000; //14,000,000 est. minute
    totalTime += timeDelta; //MAIN_LOOP_TIMER_INTERVAL;
    timeInMin += timeDelta / 60.0;
//    RTCC_TimeGet(&previousTime); //(double)MAIN_LOOP_TIMER_INTERVAL / 60.0;
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
            
//            SetAltitude(0.0);
        }
    }    
    
}

void SetTime(double formattedTime) {
    
    // 9h 56min 24sec . 00
    int hours = (formattedTime / 10000) * 60;   // hours = 9h * 60 min/h
    formattedTime = (int)formattedTime % 10000;                     // formattedTime = 56min 24sec
    int min = formattedTime / 100;              // min = 56min
    formattedTime = (int)formattedTime % 100;                       // formattedTime = 24sec
    double sec = formattedTime / 60.0;            // sec = 0.40 min
    timeInMin = hours + min + sec;
}

void SetAltitude(double alt) {
    
//     lastAltitude -= 0.25;
    lastAltitude = alt;
}

time_t logCount = 1;

// Logging Method for Diagnostic Mode
void LogState() {
    
    char log[1100] = "UNITE Log #";
    char newLine[] = "\n\n";
    char endLine[] = "EOF\n";
    
    //Log Count
    char count[10];
    sprintf(count, "%u", (unsigned int)logCount++);
    strcat(log, count);
    
    strcat(log, newLine);
    
    //Current Mode

    switch (currentMode) {
        case firstWeek: strcat(log, "Current Mode: firstWeek"); break;
        case interim: strcat(log, "Current Mode: interim"); break;
        case stabilize: strcat(log, "Current Mode: stabilize"); break;
        case science: strcat(log, "Current Mode: science"); break;
        case reentry: strcat(log, "Current Mode: reentry"); break;
        case fallback: strcat(log, "Current Mode: fallback"); break;
        default: break;
    }
    
    strcat(log, newLine);
    
    //Altitude
    char altitude[20];
    sprintf(altitude, "Altitude: %.2f km", lastAltitude);
    strcat(log, altitude);
    
    strcat(log, newLine);
    
    //Time 
    char timeString[65];
    sprintf(timeString, "Total runtime: %u h %2d min\nTime of day: %d:%2d UTC\nEpoch: %u", (unsigned int)(totalTime / 3600), ((int)(totalTime / 60) % 60), (int)(timeInMin / 60.0),(int)timeInMin % 60,(unsigned int)duplexEpoch);
    strcat(log, timeString);
    
    strcat(log, newLine);
    
    // Queue
    char queueString[30];
    sprintf(queueString, "Transmit Queue Size: %d", (int)transmitQueueLength);
    strcat(log, queueString);
    
    strcat(log, newLine);
    
    // Conversions
    double convertedMagDiagData[3];
    double convertedTempDiagData[8];
    
    // Magnetometer
    int i;
    for (i = 0; i < 3; i++) {
        convertedMagDiagData[i] = ((double)magnetometerDiagData[i]*(4.0 / 1023.0) - 2.0) * 100000.0;
    }
    
    // Temperature
    for (i = 0; i < 8; i++) {

        double convertedTemp, unconvertedTemp = (double) temperatureDiagData[i];


        switch (i) {
            case 0: // MAG
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //(0.178813745211130 * unconvertedTemp) - 51.4007; //0.80691 * (unconvertedTemp + 270.1) - 272.67;
                break;
            case 1: // -Z
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80405 * (unconvertedTemp / 4.0 + 271.6) - 273.19;
                break;
            case 2: // -Y
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80528 * (unconvertedTemp / 4.0 + 270.88) - 273.18;
                break;
            case 3: // -X
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80697 * (unconvertedTemp / 4.0 + 270.3) - 273.63;
                break;
            case 4: // +Z
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.81446 * (unconvertedTemp / 4.0 + 267.89) - 273.51;
                break;
            case 5: // +Y
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80379 * (unconvertedTemp / 4.0 + 271.37) - 273.71;
                break;
            case 6: // +X
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80103 * (unconvertedTemp / 4.0 + 272.5) - 273.7;
                break;
            case 7: // CMD
                convertedTemp = 0.24771 * (unconvertedTemp) - 79.82089; //0.80887 * (unconvertedTemp / 4.0 + 268.74) - 274.05;
                break;
        }

        convertedTempDiagData[i] = convertedTemp;
    }
    
    //Housekeeping
    char diagData[900];
    sprintf(diagData,
            "Battery 1 Charge: %u \nBattery 2 Charge: %u \nBattery 1 Voltage: %u \nBattery 2 Voltage: %u \nBattery 1 Current: %u \nBattery 2 Current: %u \nBuss+ Voltage: %u \nSolar Panel 1 Voltage: %u \nSolar Panel 2 Voltage: %u \nSolar Panel 3 Voltage: %u \nSolar Panel 4 Voltage: %u \nSimplex Temp: %u \nDuplex Temp: %u \nEPS Temp: %u \n\nLP Temp: %u \nLP Cal: %u, %u, %u, %u \nMagnetometer x: %.2f nT (%u), y: %.2f nT (%u), z: %.2f nT (%u)\nTemperature: MAG: %.2f C (%u), -Z: %.2f C (%u), -Y: %.2f C (%u), -X: %.2f C (%u),\n\t+Z: %.2f C (%u), +Y: %.2f C (%u), +X: %.2f C (%u), CMD: %.2f C (%u)\nGPS error code: %d, datum: %u\nGPS altitude: %.2f\nGPS Lock Attempts: %u", //GPS Position x: %.2f, y: %.2f, z: %.2f\nGPS Velocity x: %.3f, y: %.3f, z: %.3f
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
            (double)convertedMagDiagData[0],
            (unsigned int)magnetometerDiagData[0],
            (double)convertedMagDiagData[1],
            (unsigned int)magnetometerDiagData[1],
            (double)convertedMagDiagData[2],
            (unsigned int)magnetometerDiagData[2],
            (double)convertedTempDiagData[0],
            (unsigned int)temperatureDiagData[0],
            (double)convertedTempDiagData[1],
            (unsigned int)temperatureDiagData[1],
            (double)convertedTempDiagData[2],
            (unsigned int)temperatureDiagData[2],
            (double)convertedTempDiagData[3],
            (unsigned int)temperatureDiagData[3],
            (double)convertedTempDiagData[4],
            (unsigned int)temperatureDiagData[4],
            (double)convertedTempDiagData[5],
            (unsigned int)temperatureDiagData[5],
            (double)convertedTempDiagData[6],
            (unsigned int)temperatureDiagData[6],
            (double)convertedTempDiagData[7],
            (unsigned int)temperatureDiagData[7],
            /*(double)gpsPosition[0],
            (double)gpsPosition[1],
            (double)gpsPosition[2],
            (double)gpsVelocity[0],
            (double)gpsVelocity[1],
            (double)gpsVelocity[2],*/
            (int)gpsError,
            (unsigned int)gpsDatum,
            (double)gpsAltitude,
            (unsigned int)gpsLockAttempts);
    
    strcat(log, diagData);
    
    strcat(log, newLine);
    strcat(log, endLine);
    
    uint16_t length = strlen(log);
    for (i = 0; i < length; i++) {
        Send((uint8_t)(log[i]), DiagUnit);
    }
    
}