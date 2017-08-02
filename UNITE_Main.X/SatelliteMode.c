#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
UNITEMode currentMode = safe;
bool shouldChangeMode = false;
unsigned long lastAltitude = 400000;
unsigned long totalTime = 0; // Keeps track of overall mission clock
double timeInMin = 0.0;      // Time in Min since 00:00

bool isDuplexConnected = true;

// CONSTANTS
uint16_t INTERIM_STOP_TIME = 21600; // 1800; // 30 mins in
uint16_t SCIENCE_STOP_TIME = 43200; //3600;  // 60 mins in
uint16_t REENTRY_STOP_TIME = 52000; //10800n // 3 hrs in

/*******************************
  Satellite Mode Configurations
 *******************************/

SatelliteMode InterimMode = {
    400, // Altitude to begin sampling in this mode 
    300, // Altitude to end sampling and switch to new mode
    380, // Days until needs to switch mode
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
};

/******************************
 Satellite Mode Initializations
 ******************************/

void SatelliteProperties_Initialize() {
    // Set Current Mode to interim on start
    currentMode = UpdateMode();
    TMR1_INTERRUPT_TICKER_FACTOR = 1;
    
    // Initialize Debug light to on
   _LATE1 = LED_ON;
   _LATE2 = LED_ON;
   
   _LATE3 = LED_OFF;
   _LATE4 = LED_OFF;
   
   //Slave select 1
   _RG9 = 1;            
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

bool ShouldUpdateMode(unsigned long time, unsigned long altitude) {

    // Check current altitude
    switch (currentMode) {
        case safe: return true;
        case interim:
            if (time > InterimMode.stopTime) return true;
            else if ((altitude / 1000) <= InterimMode.endAltitudeInKm) return true;
        case science:
            if (time > ScienceMode.stopTime) return true;
            else if ((altitude / 1000) <= ScienceMode.endAltitudeInKm) return true;
        case reentry: break;
            if (time > ReEntryMode.stopTime) return true;
            else if ((altitude / 1000) <= ReEntryMode.endAltitudeInKm) return true;
    }
    return false;
}

UNITEMode UpdateMode() {
    // Determine which mode should be active

    if (ShouldUpdateMode(totalTime, lastAltitude)) {
        
        /*
        int i;
        for (i = 0; i < 8; i++) {
            UART3_Write(255); 
            UART3_Write(0);
        }*/
        
        switch (currentMode) {
            case interim:

                _LATE2 = LED_OFF;
                _LATE3 = LED_ON;

                return science;
            case science:

                _LATE3 = LED_OFF;
                _LATE4 = LED_ON;

                return reentry;
            case reentry:

                _LATE4 = LED_OFF;

                TMR5_Stop();

                return safe;

            case safe:

                return interim;
        }
    }
    
    return currentMode;
}

/*******************
  Controller Method
 *******************/

void MainLoop() {
    
    // Langmuir Probe
    if (!isLangmuirProbeSampling) {
        if (++currentLangmuirProbeWait >= GetSampleRate(&LangmuirProbe)) {

            if (isLangmuirProbeOn()) {
                BeginLangmuirProbeSampling();
                currentLangmuirProbeWait = 0;
            } else {
                SetLangmuirProbePower(1);
            }
        } // Wait to check MAG before turning switch off
    } 
    
    // Magnetometer
    if ((++currentMagnetometerWait >= GetSampleRate(&Magnetometer)) || shouldMagnetometerSample) {
        
        if (isMagnetometerOn()) {
            BeginMagnetometerSampling();
            currentMagnetometerWait = 0;
        } else {
            SetMagnetometerPower(1);
        }
    } else SetMagnetometerPower(0);
    
    
    // Temperature Sensors
    if (++currentTemperatureWait >= GetSampleRate(&TemperatureSensors)) {
        
        if (isTemperatureOn()) {
            BeginTemperatureSampling();
            currentTemperatureWait = 0;
        } else {
            SetTemperaturePower(1);
        }
    } else SetTemperaturePower(0);
    
    // GPS
    if (++currentGPSWait >= GetSampleRate(&GPS)) {
        
        if (isGPSOn()) {
            BeginGPSSampling();
            currentGPSWait = 0;
        } else {
            SetGPSPower(1);
        }
        
    } else SetGPSPower(0);
    
    
    TogglePowerSwitches();
        
    // Transmission
    TransmitQueue();
 
    // Update Time    
    totalTime += TMR5_INTERRUPT_TICKER_FACTOR;
    timeInMin += (double)TMR5_INTERRUPT_TICKER_FACTOR / 60.0;
    
    // Update SatelliteMode
    currentMode = UpdateMode();
    
//    TestDACSPI();
    
    // Commanding
    HandleCommand();
}


void SetTime(uint8_t *time, int arrayLength) {

    int hours = 0;
    int mins = 0;
    
    // Convert string to int    
    int i;
    for (i = 0; i < arrayLength; i++) {
        switch (i) {
            case 0: 
                hours += time[i] & 0x0F;
                hours += ((time[i] & 0xF0) >> 4) * 10;
                break;
            case 1: 
                mins += time[i] & 0x0F;
                mins += ((time[i] & 0xF0) >> 4) * 10;
                break;
            default: break;
        }
    }
    
    timeInMin = ((hours * 60) + mins);
}

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
}