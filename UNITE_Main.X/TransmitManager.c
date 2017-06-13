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
#include "mcc_generated_files/uart2.h"
#include "mcc_generated_files/uart3.h"


/***********************
  Transmission Package 
 ***********************/
bool isSending = false;
uint8_t transmitQueue[2000];
int transmitQueueStartIndex = 0;
int transmitQueueLength = 0;
const int HEADER_SIZE = 5;
const uint16_t QUEUE_SIZE = 2000;

// Simplex Comm Configs
const int PREAMBLE_LENGTH = 4;
uint8_t simplexPackagePreamble[4] = {80, 80, 80, 12}; // 0x50 0x50 0x50 0x0C
const int SIM_RES_LENGTH = 3;

// Duplex Comm Configs
const int DUPLEX_SYNC_LENGTH = 2;
const uint8_t duplexSyncBytes[2] = {71, 85}; // 0x47, 0x55
const int DUPLEX_PACK_BYTE_LENGTH = 4;
const char duplexUplinkFilePoll[] = "PR222";
const char duplexUplinkFileReady[] = "RR222";
const char duplexDownlinkFilePoll[] = "PP333";
const char duplexDownlinkFileReady[] = "RP333";
const char duplexHousekeepingFilePoll[] = "PC401";
const char duplexWaitingFilesPoll[] = "PC403";
const int DUPLEX_TIMEOUT = 10;

const int DUP_RES_LENGTH = 11;

/*************************
  Power Switch Properties
 *************************/

uint8_t powerPackagePreamble[4] = {80, 80, 80, 11}; // 0x50 0x50 0x50 0x0B
const uint8_t commandBoardPowerSwitch = 0xFF;   // SW1
uint8_t temperaturePowerSwitch = 0xFF;          // SW2
uint8_t langmuirMagPowerSwitch = 0xFF;          // SW3
uint8_t gpsPowerSwitch = 0xFF;                  // SW4
uint8_t duplexPowerSwitch = 0xFF;               // SW5

/********************
 Data Manager Methods
 ********************/

// Private Source Methods
bool IsLineBusy(TransmissionUnit unit);
void SendData(uint8_t *queue, int queueLength);
void TransmitInstrumentDataToSimplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength);
bool TransmitInstrumentDataToDuplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength);
void PollDuplex(const char *pollCode, uint16_t taggedDataLength);
bool ReadACKForUnit(TransmissionUnit unit);
uint16_t GetWaitingFilesCount();
void GetCommandFile();

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
    package[1] = time >> 8;
    package[2] = time & 0x00FF;
    package[3] = bufferSize >> 8;
    package[4] = bufferSize & 0x00FF;
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        package[HEADER_SIZE + i] = buffer[i];
    }
    
    SaveData(package, bufferSize + HEADER_SIZE);    
    return i;
}

/*********************
  Simplex/Duplex Comm
 *********************/

void TransmitQueue() {
    if (!isSending && transmitQueueLength > 0) {
        
        if (GetTransmissionUnitForMode() > 0) {
            if (duplexPowerSwitch == 0) PowerOnDuplex();
            else SendData(transmitQueue, transmitQueueLength);
        } else {
            SendData(transmitQueue, transmitQueueLength);
        }
    }
    
    if (!isSending && duplexPowerSwitch > 0 && GetWaitingFilesCount() == 0) {
        PowerOffDuplex();
    }
}

void Send(uint8_t byte, TransmissionUnit unit) {
    
    while (IsLineBusy(unit));
    switch (unit) {
        case SimplexUnit: UART3_Write(byte); break;
        case DuplexUnit: UART2_Write(byte); break;
        default: break;
    }
}

uint8_t Read(TransmissionUnit unit) {
    
    // Might not need to check if line is busy
    while (IsLineBusy(unit));
    switch (unit) {
        case SimplexUnit: return UART3_Read();
        case DuplexUnit: return UART2_Read();
        default: return 0xFF;
    }
}

/**********************
  Transmission Headers
 **********************/

void TransmitPreamble(uint8_t *preamble) {
    int i;
    for (i = 0; i < PREAMBLE_LENGTH; i++) {
        Send(preamble[i], SimplexUnit);
        wait_for(10);
    }
 }

void PollDuplex(const char *pollCode, uint16_t taggedDataLength) {
    
    int HEADER_BYTE_LENGTH = DUP_RES_LENGTH - DUPLEX_SYNC_LENGTH;
    
    uint8_t packageLengthHigh = (HEADER_BYTE_LENGTH + taggedDataLength) >> 8;
    uint8_t packageLengthLow = (HEADER_BYTE_LENGTH + taggedDataLength) & 0x00FF;
        
    int i;
    for (i = 0; i < DUP_RES_LENGTH; i++) {
        
        switch (i) {
            case 0: Send(duplexSyncBytes[i], DuplexUnit); break;
            case 1: Send(duplexSyncBytes[i], DuplexUnit); break;
            case 2: Send(0, DuplexUnit); break;
            case 3: Send(0, DuplexUnit); break;
            case 4: Send(packageLengthHigh, DuplexUnit); break;
            case 5: Send(packageLengthLow, DuplexUnit); break;
            default: Send(pollCode[i % 6], DuplexUnit); break;
        }
    }
}

/**********************
  Transmission Methods
 **********************/

// Description: Handler for sending data via UART
// *dataString => pointer to a string of packaged data
// stringLength => length of packaged data string

void SendData(uint8_t *queue, int queueLength) {
    
    bool isError = false;
    
    isSending = true;
    
    while (queueLength > 0) {
        // System Header Parser
        uint8_t *sysID = (uint8_t *)&queue[transmitQueueStartIndex];
        uint8_t *timeH = (uint8_t *)&queue[(transmitQueueStartIndex + 1)%QUEUE_SIZE];
        uint8_t *timeL = (uint8_t *)&queue[(transmitQueueStartIndex + 2)%QUEUE_SIZE];
        uint16_t dataLength = (queue[(transmitQueueStartIndex + 3)%QUEUE_SIZE] << 8) | queue[(transmitQueueStartIndex + 4)%QUEUE_SIZE];
        
        if (dataLength > QUEUE_SIZE) {
            
            // Error : Reset Queue
            isError = true;
            
            ClearQueue(transmitQueue, QUEUE_SIZE, 0);
            
            transmitQueueStartIndex = 0;
            transmitQueueLength = 0;
            queueLength = 0;
            dataLength = 0;
        }
        
        uint8_t headerByte1 = *sysID + *timeH;
        uint8_t headerByte2 = *timeL;
        
        free(sysID);
        free(timeH);
        free(timeL);
        
        if (!isError) {
            
            switch (GetTransmissionUnitForMode()) {
                case SimplexUnit: TransmitInstrumentDataToSimplex(queue, headerByte1, headerByte2, dataLength);
                    break;
                case DuplexUnit: TransmitInstrumentDataToDuplex(queue, headerByte1, headerByte2, dataLength);
                    break;
                case SimplexOrDuplex:
                    if (!TransmitInstrumentDataToDuplex(queue, headerByte1, headerByte2, dataLength)) {
                        TransmitInstrumentDataToSimplex(queue, headerByte1, headerByte2, dataLength); }
                    break;
            }

            queueLength = max(queueLength - HEADER_SIZE - dataLength, 0);
        }
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

void TransmitInstrumentDataToSimplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength) {
    
    TransmitPreamble(simplexPackagePreamble);
    
    Send(headerByte1, SimplexUnit);
    wait_for(10);
    Send(headerByte2, SimplexUnit);
    wait_for(10);

    // Update Queue position properties
    ClearQueue(transmitQueue, HEADER_SIZE, transmitQueueStartIndex);
    transmitQueueStartIndex = (transmitQueueStartIndex + HEADER_SIZE) % QUEUE_SIZE;
    transmitQueueLength = max(transmitQueueLength - HEADER_SIZE, 0);

    // Send data in packages
    int i = 0;
    while (i < dataLength) {


        // One package transmission
            int j;
            uint16_t packageLength;
            
            // Package is shortened to make room for header
            if (i == 0) packageLength = GetTransmissionPackageLength(SimplexUnit) - 2;
            else {
                TransmitPreamble(simplexPackagePreamble);
                packageLength = GetTransmissionPackageLength(SimplexUnit);
            }

            // Sends enough data for a package
            for (j = 0; j < packageLength; j++) {

                // If package space exceeds instrument's data, fill with 0
                if (j >= dataLength - i) Send(0, SimplexUnit);
                // Send next data byte 
                else Send(queue[(transmitQueueStartIndex + j) % QUEUE_SIZE], SimplexUnit);
                
                wait_for(10);
            }

            //            if (ReadACKForUnit(currentTransmissionUnit)) {
            if (true) {
                ClearQueue(transmitQueue, min(packageLength, dataLength - i), transmitQueueStartIndex);
                transmitQueueStartIndex = (transmitQueueStartIndex + min(j, dataLength - i)) % QUEUE_SIZE;
                transmitQueueLength = max(transmitQueueLength - j, 0);
                i = i + j;
            }

    }
}

bool TransmitInstrumentDataToDuplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength) {

    int i, duplexTimeoutCounter;
    while (i < dataLength) {

        PollDuplex(duplexDownlinkFilePoll, 0);

        if ((ReadACKForUnit(DuplexUnit)) && (duplexTimeoutCounter++ < DUPLEX_TIMEOUT)) {

            // One package transmission
            PollDuplex(duplexDownlinkFileReady, GetTransmissionPackageLength(DuplexUnit));
            
            int j;
            uint16_t packageLength;
            
            // First package is shortened to make room for header
            if (i == 0) {
                Send(headerByte1, DuplexUnit);
                wait_for(10);
                Send(headerByte2, DuplexUnit);
                wait_for(10);
                packageLength = GetTransmissionPackageLength(DuplexUnit) - 2;
            }
            else {
                packageLength = GetTransmissionPackageLength(DuplexUnit);
            }

            // Sends enough data for a package
            for (j = 0; j < packageLength; j++) {

                // If package space exceeds data buffer, fill with 0
                if (j >= dataLength - i) Send(0, DuplexUnit);
                // Send next data byte 
                else Send(queue[(transmitQueueStartIndex + j) % QUEUE_SIZE], DuplexUnit);

                wait_for(10);
            }

            if (ReadACKForUnit(DuplexUnit)) {
                ClearQueue(transmitQueue, min(packageLength, dataLength - i), transmitQueueStartIndex);
                transmitQueueStartIndex = (transmitQueueStartIndex + min(j, dataLength - i)) % QUEUE_SIZE;
                transmitQueueLength = max(transmitQueueLength - j, 0);
                i = i + j;
                duplexTimeoutCounter = 0;
            }
        } 
        else if (duplexTimeoutCounter > DUPLEX_TIMEOUT) return false;
    }
    
    return true;
}

/***************************************
  Transmission Unit Interfacing Methods
 ***************************************/

bool ReadACKForUnit(TransmissionUnit unit) {
    
    int i;
    bool isACK = true;

    switch (unit) {
        case SimplexUnit: 
            for (i=0; i < SIM_RES_LENGTH; i++) {
                switch (i) {
                    case 3: isACK = (Read(unit) == 0); break;
                    default: Read(unit); break;
                }
            }
            break;
        case DuplexUnit:
            for (i=0; i < DUP_RES_LENGTH; i++) {
                switch (i) {
                    case 6: isACK = (Read(unit) == 6); break;
                    default: Read(unit); break;
                }
            }
            break;
        default: break;
    }
    
    return isACK;
}

bool IsLineBusy(TransmissionUnit unit) {
    return false;
}

uint16_t GetWaitingFilesCount() {
    
    PollDuplex(duplexWaitingFilesPoll, 0);
    
    if (ReadACKForUnit(DuplexUnit)) {
        
        int i;
        int responseLength = 15;
        
        uint8_t highByte, lowByte;
        
        for (i = 0; i < responseLength; i++) {
            
            switch (i) {
                case 14: highByte = Read(DuplexUnit); break;
                case 15: lowByte = Read(DuplexUnit); break;
                default: Read(DuplexUnit); break;
            }
        }
        
        return (highByte << 8) | lowByte;
    }
    
    return -1;
}

void GetCommandFile() {
    
    PollDuplex(duplexUplinkFilePoll, 0);
    
    if (ReadACKForUnit(DuplexUnit)) {
        
        // Need to make sure that the command string is a set number of bytes
        int i;
        int fileResponseLength = 65;
        
        uint8_t commandString[56];
        for (i = 0; i < fileResponseLength; i++) {
            
            if (i >= DUP_RES_LENGTH) { commandString[i - DUP_RES_LENGTH] = Read(DuplexUnit); }
            else Read(DuplexUnit);
        }
        
        PerformCommands(commandString, 56);
    }
}   

/*************************
   Power Manager Methods
 *************************/

bool TogglePowerSwitches() {
    
    if (!isSending) {
        
        TransmitPreamble(powerPackagePreamble);
                
        uint8_t powerPackage[10] = {0x01, commandBoardPowerSwitch, 
                                    0x02, temperaturePowerSwitch, 
                                    0x03, langmuirMagPowerSwitch,
                                    0x04, gpsPowerSwitch,
                                    0x05, duplexPowerSwitch};
                
        int i;
        for (i=0; i < 35; i++) {
            if (i < 10) { Send(powerPackage[i], SimplexUnit); }
            else { Send(0, SimplexUnit); }
        }
        
        return ReadACKForUnit(SimplexUnit);
    }
    
    return false;
}

bool PowerOffDuplex() {
    
    duplexPowerSwitch = 0x00;

    return TogglePowerSwitches();
}

bool PowerOnDuplex() {
    
    duplexPowerSwitch = 0xFF;

    return TogglePowerSwitches();
}

