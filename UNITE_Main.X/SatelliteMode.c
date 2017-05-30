#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include <stdlib.h>
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

unsigned long totalTime = 0; // Keeps track of overall mission clock

// CONSTANTS
uint16_t INTERIM_STOP_TIME = 1800; // 30 mins in
uint16_t SCIENCE_STOP_TIME = 3600; // 60 mins in
uint16_t REENTRY_STOP_TIME = 10800; // 3 hrs in

/*******************************
  Satellite Mode Configurations
 *******************************/

SatelliteMode InterimMode = {
    400, // Altitude to begin sampling in this mode 
    300, // Altitude to end sampling and switch to new mode
};

SatelliteMode ScienceMode = {
    300,
    200,
};

SatelliteMode ReEntryMode = {
    200,
    0,
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
    currentMode = interim;
    TMR1_INTERRUPT_TICKER_FACTOR = 1;
}

 
void Satellite_Initialize() {

    // Initialize Analog/Digital Converter
    InitializeADC1();
    
    // Initialize Main Mode Timer
    TMR5_Initialize();
    
    // Initialize main variables
    SatelliteProperties_Initialize();
    
    // Start Main Timer
    TMR5_Start();
        
    // Debug light
    _LATE2 = LED_ON;
}

/********************
 Mode Switch Handling
 ********************/

// Description: Handler for switching satellite modes

UNITEMode UpdateMode() {
    // Determine which mode should be active
    // Return value of mode
    int i;
    for (i = 0; i < 8; i++) {
        UART3_Write(255); //Will tell us it is in interim mode 
        UART3_Write(0);
    }    
    switch (currentMode) {
        case interim:

            // Switch Timers
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
            
        default:

            return safe;
    }
}

bool ShouldUpdateMode(unsigned long time) {

    // Check current altitude

    
    // After 30 min switch from interim to science mode
    if ((time == INTERIM_STOP_TIME) || ((currentMode == interim) && (time > INTERIM_STOP_TIME))) {
        return true;

    // After 105 min switch from science to reentry mode
    } else if ((time == SCIENCE_STOP_TIME) || ((currentMode == science) && (time > SCIENCE_STOP_TIME))) {
        return true;

    // After 165 min switch from reentry to safe mode and end loop
    } else if ((time == REENTRY_STOP_TIME) || ((currentMode == reentry) && (time > REENTRY_STOP_TIME))) {
        return true;
        
    } else {
        return false;
    }
}

/*******************
  Controller Method
 *******************/

void TakeSample() {

    // Langmuir Probe
    if (!isLangmuirProbeSweeping) {
        if (currentLangmuirProbeWait++ >= GetSampleRate(&LangmuirProbe)) {
            BeginLangmuirProbeSampling();
            currentLangmuirProbeWait = 0;
        }
    }
    
    // Magnetometer
    if (!isMagnetometerSweeping) {
        if (currentMagnetometerWait++ >= GetSampleRate(&Magnetometer)) {
            BeginMagnetometerSampling();
            currentMagnetometerWait = 0;
        }
    }
    
    // Temperature Sensors
    if (currentTemperatureWait++ >= GetSampleRate(&TemperatureSensors)) {
        BeginTemperatureSampling();
        currentTemperatureWait = 0;
    }
    
    // GPS
    if (currentGPSWait++ >= GetSampleRate(&GPSUnit)) {
        BeginGPSSampling();
        currentGPSWait = 0;
    }
    
    TransmitQueue();
 
    // Mode Update Test    
    if (ShouldUpdateMode(++totalTime)) {
        currentMode = UpdateMode();
    }

}
