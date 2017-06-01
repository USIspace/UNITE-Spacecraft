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
uint8_t transmitQueue[1000];
int transmitQueueStartIndex = 0;
int transmitQueueLength = 0;
const int HEADER_SIZE = 5;
const uint16_t QUEUE_SIZE = 1000;

// Simplex Comm Configs
const int SIMPLEX_PREAMBLE_LENGTH = 4;
const uint8_t powerPackagePreamble[4] = {80, 80, 80, 11}; // 0x50 0x50 0x50 0x0B
const uint8_t simplexPackagePreamble[4] = {80, 80, 80, 12}; // 0x50 0x50 0x50 0x0C
const int SIM_RES_LENGTH = 3;
const uint8_t simplexACKResponse[3] = {170, 5, 0}; // 0xAA 0x05 0x00
const uint8_t simplexNAKResponse[3] = {170, 5, 255}; // 0xAA 0x05 0xFF

// Duplex Comm Configs
const int DUPLEX_SYNC_LENGTH = 2;
const uint8_t duplexSyncBytes[2] = {71, 85}; // 0x47, 0x55
const int DUP_RES_LENGTH = 11;
const uint8_t duplexACKResponse[11] = {71, 85, 0, 0, 0, 0, 6, 0, 0, 0, 0}; // 0x4755LLLLLLLL06CCCCCCCC
const uint8_t duplexNAKResponse[11] = {71, 85, 0, 0, 0, 0, 15, 0, 0, 0, 0}; // 0x4755LLLLLLLL0FCCCCCCCC


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
        SendData(transmitQueue, transmitQueueLength);
    }
}

void Send(uint8_t byte, TransmissionUnit unit) {
    
    while (IsLineBusy(unit));
    switch (unit) {
        case SimplexOnly: UART3_Write(byte); break;
        case DuplexOnly: UART2_Write(byte); break;
        default: break;
    }
}

uint8_t Read(TransmissionUnit unit) {
    
    // Might not need to check if line is busy
    while (IsLineBusy(unit));
    switch (unit) {
        case SimplexOnly: return UART3_Read();
        case DuplexOnly: return UART2_Read();
        default: return 0xFF;
    }
}

void TransmitSimplexPreamble() {
    int i;
    for (i = 0; i < SIMPLEX_PREAMBLE_LENGTH; i++) {
        Send(simplexPackagePreamble[i], SimplexOnly);
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
        uint8_t *timeH = (uint8_t *)&queue[(transmitQueueStartIndex + 1)%QUEUE_SIZE];
        uint8_t *timeL = (uint8_t *)&queue[(transmitQueueStartIndex + 2)%QUEUE_SIZE];
        uint16_t dataLength = (queue[(transmitQueueStartIndex + 3)%QUEUE_SIZE] << 8) | queue[(transmitQueueStartIndex + 4)%QUEUE_SIZE];
        
        if (dataLength > QUEUE_SIZE) {
            while (1);
        }
        
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
        
        // Update Queue position properties
        ClearQueue(transmitQueue, HEADER_SIZE, transmitQueueStartIndex);
        transmitQueueStartIndex = (transmitQueueStartIndex + HEADER_SIZE)%QUEUE_SIZE;
        transmitQueueLength = max(transmitQueueLength - HEADER_SIZE, 0);
        
        // Send data in packages
        int i = 0;
        while (i < dataLength) {
            
            
            // One package transmission
            int j;
            uint16_t packageLength;
            if (i==0) {
                // Package is shortened to make room for header
                packageLength = GetTransmissionPackageLength(currentTransmissionUnit) - 2;
            } 
            else 
            {
                TransmitSimplexPreamble();
                packageLength = GetTransmissionPackageLength(currentTransmissionUnit);
            }
            
            // Sends enough data for a package
            for (j = 0; j < packageLength; j++) {
                
                // If package space exceeds instrument's data, fill with 0
                if (j >= dataLength - i) { Send(0, currentTransmissionUnit); }
                // Send next data byte 
                else { Send(queue[(transmitQueueStartIndex + j)%QUEUE_SIZE], currentTransmissionUnit); }
                wait_for(10);
            }
            
//            if (ReadACKForMode(currentTransmissionUnit)) {
            if (true) {
                ClearQueue(transmitQueue, min(packageLength, dataLength - i), transmitQueueStartIndex);
                transmitQueueStartIndex = (transmitQueueStartIndex + min(j, dataLength - i)) % QUEUE_SIZE;
                transmitQueueLength = max(transmitQueueLength - j, 0);
                i = i + j;
            }
            
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

bool ReadACKForMode(TransmissionUnit unit) {
    
    int i;

    switch (unit) {
        case SimplexOnly: 
            for (i=0; i < SIM_RES_LENGTH; i++) {
                if (Read(unit) == simplexNAKResponse[i]) { return false; }
            }
            break;
        case DuplexOnly:
            for (i=0; i < DUP_RES_LENGTH; i++) {
                if (i < DUPLEX_SYNC_LENGTH) { 
                    if (Read(unit) != duplexSyncBytes[i]) { return false; }
                } else if (i == 6) {
                    if (Read(unit) == duplexNAKResponse[i]) { return false; }
                } else {
                    Read(unit);
                }
            }
            break;
        default: break;
    }
    
    return true;
}

bool IsLineBusy(TransmissionUnit unit) {
    return false;
}