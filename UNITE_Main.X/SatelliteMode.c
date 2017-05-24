#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SatelliteMode.h"
#include "SampleManager.h"
#include "SystemConfiguration.h"
#include "adc1.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
#include "mcc_generated_files/uart3.h"
#include "mcc_generated_files/uart4.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/tmr3.h"
#include "mcc_generated_files/tmr4.h"
#include "mcc_generated_files/tmr5.h"
#include "mcc_generated_files/spi2.h"
#include "time.h"

/******************************
 Satellite Mode Initializations
 ******************************/

/*******************
  Global Variables
 *******************/

// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
UNITEMode currentMode = safe;
bool shouldChangeMode = false;

unsigned long totalTime = 0; // Keeps track of overall mission clock

/***********************
  Transmission Package 
 ***********************/
bool isSending = false;
uint8_t transmitQueue[2000];
uint16_t transmitQueueStartIndex = 0;
uint16_t transmitQueueLength = 0;
const uint16_t QUEUE_SIZE = 2000;

const int PREAMBLE_LENGTH = 4;
const uint8_t powerPackagePreamble[4] = {0x50, 0x50, 0x50, 0x0B};
const uint8_t simplexPackagePreamble[4] = {0x50, 0x50, 0x50, 0x0C};

// CONSTANTS
unsigned long INTERIM_STOP_TIME = 1800; // 30 mins in
unsigned long SCIENCE_STOP_TIME = 3600; // 60 mins in
unsigned long REENTRY_STOP_TIME = 10800; // 3 hrs in

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
 Data Manager Methods
 ********************/

// Description: Saves data to a transmission queue for sending later
// *package => ready-to-send package
// packageSize => size of package to send

void SaveData(uint8_t *package, uint16_t packageSize) {

    int i;
    for (i = 0; i < packageSize; i++) {
        transmitQueue[(transmitQueueStartIndex + i) % QUEUE_SIZE] = package[i];
    }
    
    transmitQueueStartIndex = (transmitQueueStartIndex + i) % QUEUE_SIZE;
    transmitQueueLength = transmitQueueLength + packageSize;
}

// Description: Packaging algorithm for collected data
// system => determines which system sampled the data
// time => time the sampling began
// *buffer => pointer to an array of instrument data
// bufferSize => length of package string in characters

uint16_t PackageData(System system, uint16_t time, uint8_t *buffer, uint16_t bufferSize) {

    int headerSize = 5;
    uint8_t package[bufferSize + headerSize];
    
    // Setup Package Header
    package[0] = GetSystemHeaderID(system);
    package[1] = time >> 4;
    package[2] = time;
    package[3] = bufferSize >> 4;
    package[4] = bufferSize;
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        package[headerSize + i] = buffer[i];
    }
    
    SaveData(package, bufferSize + headerSize);

    return i;
}

// Description: Handler for sending data via UART
// *dataString => pointer to a string of packaged data
// stringLength => length of packaged data string

void SendData(uint8_t *queue, uint16_t startIndex, uint16_t queueLength) {
    
    int i;
    for (i = 0; i < PREAMBLE_LENGTH; i++) {
        UART3_Write(simplexPackagePreamble[i]);
    }
    
    while (queueLength > 0) {
        // System Header Parser
        uint8_t sysID = queue[startIndex];
        uint8_t timeH = queue[startIndex + 1];
        uint8_t timeL =queue[startIndex + 2];
        uint16_t dataLength = queue[startIndex + 3] << 4 + queue[startIndex];
        
        // UART header here then proceed to send package
        
        for (i = 0; i < dataLength; i++) {
            
        }
    }
    
    
    /* DEBUGGING ONLY
    /*============================
     Save to Data Acquisition Unit
     =============================
    
    // Send data via UART here
    int i;
    for (i = 0; i < stringLength; i++) {

        UART3_Write(dataString[i]);
        
    }*/

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
        if (currentLangmuirProbeWait++ > GetSampleRate(LangmuirProbe, currentMode)) {
            BeginLangmuirProbeSampling();
            currentLangmuirProbeWait = 0;
        }
    }
    
    // Magnetometer
    if (!isMagnetometerSweeping) {
        if (currentMagnetometerWait++ > GetSampleRate(Magnetometer, currentMode)) {
            BeginMagnetometerSampling();
            currentMagnetometerWait = 0;
        }
    }
    
    // Temperature Sensors
    if (currentTemperatureWait++ > GetSampleRate(TemperatureSensors, currentMode)) {
        BeginTemperatureSampling();
        currentTemperatureWait = 0;
    }
    
    // GPS
    if (currentGPSWait++ > GetSampleRate(GPSUnit, currentMode)) {
        BeginGPSSampling();
        currentGPSWait = 0;
    }
    
    if (!isSending && transmitQueueLength > 0) {
        SendData(transmitQueue, transmitQueueLength);
    }
 
    // Mode Update Test    
    if (ShouldUpdateMode(++totalTime)) {
        currentMode = UpdateMode();
    }

}
