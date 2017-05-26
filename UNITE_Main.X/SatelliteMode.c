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
uint8_t transmitQueue[1000];
int transmitQueueStartIndex = 0;
int transmitQueueLength = 0;
const int HEADER_SIZE = 5;
const uint16_t QUEUE_SIZE = 1000;

const int PREAMBLE_LENGTH = 4;
const uint8_t powerPackagePreamble[4] = {80, 80, 80, 11};
const uint8_t simplexPackagePreamble[4] = {80, 80, 80, 12};

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

    while (isSending);
    int i;
    for (i = 0; i < packageSize; i++) {
        transmitQueue[(transmitQueueStartIndex + transmitQueueLength + i) % QUEUE_SIZE] = package[i];
    }
    
    transmitQueueLength = transmitQueueLength + packageSize;
    free(package);
}

// Description: Packaging algorithm for collected data
// system => determines which system sampled the data
// time => time in min since 00:00 that the sampling began 
// *buffer => pointer to an array of instrument data
// bufferSize => length of package string in characters

uint16_t PackageData(System system, uint16_t time, uint8_t *buffer, uint16_t bufferSize) {

    int packageSize = bufferSize + HEADER_SIZE;
    uint8_t package[packageSize];
    
    // Setup Package Header
    package[0] = GetSystemHeaderID(system);
    package[1] = time >> 4;
    package[2] = time & 0x00FF;
    package[3] = bufferSize >> 4;
    package[4] = bufferSize & 0x00FF;
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        package[HEADER_SIZE + i] = buffer[i];
    }
    
    SaveData(package, bufferSize + HEADER_SIZE);    
    return i;
}

void TransmitSimplexPreamble() {
    int i;
    for (i = 0; i < PREAMBLE_LENGTH; i++) {
        UART3_Write(simplexPackagePreamble[i]);
        wait_for(10);
    }
 }

// Description: Handler for sending data via UART
// *dataString => pointer to a string of packaged data
// stringLength => length of packaged data string

void SendData(uint8_t *queue, int queueLength) {
    
    isSending = true;
    
    while (queueLength > 0) {
        // System Header Parser
        uint8_t *sysID = (uint8_t *)&queue[transmitQueueStartIndex];
        uint8_t *timeH = (uint8_t *)&queue[transmitQueueStartIndex + 1];
        uint8_t *timeL = (uint8_t *)&queue[transmitQueueStartIndex + 2];
        uint16_t dataLength = (queue[transmitQueueStartIndex + 3] << 4) | queue[transmitQueueStartIndex + 4];
        
        uint8_t headerByte1 = *sysID + *timeH;
        uint8_t headerByte2 = *timeL;
        
        free(sysID);
        free(timeH);
        free(timeL);
        
        // UART header here then proceed to send package
        TransmitSimplexPreamble();
        UART3_Write(headerByte1);
        wait_for(10);
        UART3_Write(headerByte2);
        wait_for(10);
        
        ClearQueue(transmitQueue, HEADER_SIZE, transmitQueueStartIndex);
        transmitQueueStartIndex = (transmitQueueStartIndex + HEADER_SIZE) % QUEUE_SIZE;
        transmitQueueLength = max(transmitQueueLength - HEADER_SIZE, 0);
        int i = 0;
        while (i < dataLength) {
            
            
            // One Package Transmission
            int j;
            uint16_t packageLength;
            if (i==0) {
                packageLength = GetTransmissionPackageLength(currentTransmissionUnit) - 2;
            } 
            else 
            {
                TransmitSimplexPreamble();
//                wait_for(10);
                packageLength = GetTransmissionPackageLength(currentTransmissionUnit);
            }
            
            for (j = 0; j < packageLength; j++) {
                if (j >= dataLength - i) { UART3_Write(0); }
                else { UART3_Write(queue[(transmitQueueStartIndex + j) % QUEUE_SIZE]); }
                wait_for(10);
            }
            
            ClearQueue(transmitQueue, packageLength, transmitQueueStartIndex);
            transmitQueueStartIndex = (transmitQueueStartIndex + j) % QUEUE_SIZE;
            transmitQueueLength = max(transmitQueueLength - j, 0);
            i = i + j;
            
        }
        
//        transmitQueueStartIndex = (transmitQueueStartIndex + HEADER_SIZE + dataLength) % QUEUE_SIZE;
        queueLength = max(queueLength - HEADER_SIZE - dataLength, 0);
    }   
    
    isSending = false;
    
    /* DEBUGGING ONLY
    ============================
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
    
    if (!isSending && transmitQueueLength > 0) {
        SendData(transmitQueue, transmitQueueLength);
    }
 
    // Mode Update Test    
    if (ShouldUpdateMode(++totalTime)) {
        currentMode = UpdateMode();
    }

}
