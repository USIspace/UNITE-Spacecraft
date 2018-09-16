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
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
#include "mcc_generated_files/uart3.h"
#include "mcc_generated_files/uart4.h"
#include "mcc_generated_files/crc16.h"


/***********************
  Transmission Package 
 ***********************/
bool isSending = false;
uint8_t transmitQueue[2000];
int transmitQueueStartIndex = 0;
int transmitQueueLength = 0;
const int PACKAGE_LIMIT = 4;
const int HEADER_SIZE = 4; // 5
const int DATA_SIZE = 33;
const uint16_t QUEUE_SIZE = 2000;

// Simplex Comm Configs
const int PREAMBLE_LENGTH = 4;
uint8_t simplexPackagePreamble[] = {0x50, 0x50, 0x50, 0x0C};
const int SIM_RES_LENGTH = 3;

// Duplex Comm Configs
const int DUPLEX_SYNC_LENGTH = 2;
const uint8_t duplexSyncBytes[2] = {71, 85}; // 0x47, 0x55
const int DUPLEX_PACK_BYTE_LENGTH = 4;
const char duplexUplinkSMSPoll[] = "PR111";
const char duplexUplinkSMSReady[] = "RR111";
const char duplexCmdReceived[] = { 6, 'R', '1', '1', '1'};
const char duplexDownlinkFilePoll[] = "PP333";
const char duplexDownlinkFileReady[] = "RP333";
const char duplexHousekeepingFilePoll[] = "PC401";
const char duplexWaitingFilesPoll[] = "PC403";

// Commanding
uint8_t commandString[34] = { NULL };

// Instrument file names
const int FILE_COUNT_INDEX = 7;
char langmuirProbeFileName[] = "LAPData";
char magnetometerFileName[] = "MAGData";
char temperatureFileName[] = "TMPData";
char gpsFileName[] = "GPSData";
char housekeepingFileName[] = "EPSData";
int langmuirProbeFileCount = 1;
int magnetometerFileCount = 1;
int temperatureFileCount = 1;
int gpsFileCount = 1;
int housekeepingFileCount = 1;

// State Variables
int currentDuplexConnectionWait = 0;
const int DUPLEX_TIMEOUT = 3;
bool isDuplexConnected = false;
uint16_t waitingFilesCount = 0;

// Timeout
bool simplexTimeoutFlag = 0;
bool duplexTimeoutFlag = 0;
bool gpsTimeoutFlag = 0;

const int DUP_RES_LENGTH = 11;

/*************************
  Power Switch Properties
 *************************/

uint8_t epsEcho[39] = { NULL };
const int POWER_ECHO_LENGTH = 39;
uint8_t powerPackagePreamble[4] = {0x50, 0x50, 0x50, 0x0B}; // 0x50 0x50 0x50 0x0B
uint8_t commandBoardPowerSwitch = 0xFF;         // SW1
uint8_t temperaturePowerSwitch = 0x00;          // SW2
uint8_t langmuirMagPowerSwitch = 0x00;          // SW3
uint8_t gpsPowerSwitch = 0x00;                  // SW4
uint8_t duplexPowerSwitch = 0x00;               // SW5

bool isLangmuirProbeOn() { return langmuirMagPowerSwitch == 0xFF; }
bool isMagnetometerOn() { return langmuirMagPowerSwitch == 0xFF; }
bool isTemperatureOn() { return temperaturePowerSwitch == 0xFF; }
bool isGPSOn() { return gpsPowerSwitch == 0xFF; }
bool isDuplexOn() { return duplexPowerSwitch == 0xFF; } 

// Housekeeping Data
uint16_t b1Charge;
uint16_t b2Charge;
uint16_t b1Voltage;
uint16_t b2Voltage;
uint16_t b1Current;
uint16_t b2Current;

uint16_t bussPlusVoltage;

uint16_t solar1Voltage;
uint16_t solar2Voltage;
uint16_t solar3Voltage;
uint16_t solar4Voltage;

uint16_t simplexTemp;
uint16_t duplexTemp;
uint16_t epsTemp;
/********************
 Data Manager Methods
 ********************/

// Private Source Methods
bool IsLineBusy();
void SendData(uint8_t *queue, int queueLength, TransmissionUnit);
void TransmitInstrumentDataToSimplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength);
bool TransmitInstrumentDataToDuplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength);
void PollDuplex(const char *pollCode, uint16_t taggedDataLength);
bool ReadACKForUnit(TransmissionUnit unit);
size_t CreateFileHeader(char *formattedString,uint8_t instrument, uint16_t dataLength);
uint16_t GetWaitingFilesCount();
void HandleCommand();

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

    int i;
    int packages = bufferSize / DATA_SIZE;
    
    // Setup temporary package structure
    int packageSize = DATA_SIZE + HEADER_SIZE;
    uint8_t package[packageSize];

    // Setup Package Header
    package[0] = GetSystemHeaderID(system);
    package[1] = time >> 8;
    package[2] = time & 0x00FF;
    package[3] = DATA_SIZE;
    
    
    // If more than one packet is included
    for (i = 0; i < packages; i++) {
        
        int j;
        for (j = 0; j < DATA_SIZE; j++) {
            package[HEADER_SIZE + j] = buffer[j + i * DATA_SIZE];
        }

        SaveData(package, packageSize);
    }
    
    // Reduced size for last packet
    package[3] = bufferSize % DATA_SIZE;
    
    // If only one packet is included
    int j;
    for (j = 0; j < bufferSize % DATA_SIZE; j++) {
        package[j + HEADER_SIZE] = buffer[j + i * DATA_SIZE];
    }
    
    SaveData(package, HEADER_SIZE + bufferSize % DATA_SIZE);    
    
//    int packageSize = bufferSize + HEADER_SIZE;
//    uint8_t package[packageSize];
//    
//    // Setup Package Header
//    package[0] = GetSystemHeaderID(system);
//    package[1] = time >> 8;
//    package[2] = time & 0x00FF;
//    package[3] = bufferSize >> 8;
//    package[4] = bufferSize & 0x00FF;
//    
//    int i;
//    for (i = 0; i < bufferSize; i++) {
//        package[HEADER_SIZE + i] = buffer[i];
//    }
    
//    SaveData(package, packageSize);    
    return packages + 1;
}

/*********************
  Simplex/Duplex Comm
 *********************/

void TransmitQueue() {
    if (!isSending && transmitQueueLength > 0) {
        
        TransmissionUnit unit = GetTransmissionUnitForMode();
        waitingFilesCount = GetWaitingFilesCount();
        
        if (unit > 0 && currentDuplexConnectionWait < DUPLEX_TIMEOUT && isDuplexOn() && waitingFilesCount < MAX_DUP_FILES_WAITING) {
            if (isDuplexConnected == false) currentDuplexConnectionWait++;
            else {
                SendData(transmitQueue, transmitQueueLength, unit);
                currentDuplexConnectionWait = 0;
            }
        } else {
            SendData(transmitQueue, transmitQueueLength, SimplexUnit);
            currentDuplexConnectionWait = 0;
        }
    }
}

void Send(uint8_t byte, TransmissionUnit unit) {
        
    switch (unit) {
        case SimplexUnit: UART3_Write(byte); break;
        case DuplexUnit: UART2_Write(byte); break;
        case DiagUnit: UART4_Write(byte); break;
        default: break;
    }
}

uint8_t Read(TransmissionUnit unit) {
    
    if (!IS_DEBUG) {
        // Might not need to check if line is busy
        switch (unit) {
            case SimplexUnit: 
                
                // Reset Timeout
                simplexTimeoutFlag = false;
                
                uint8_t simData = UART3_Read();
                
                if (!simplexTimeoutFlag) return simData;
                else return 0;
                
            case DuplexUnit: 
                
                // Reset Timeout
                duplexTimeoutFlag = false;
                
                uint8_t dupData = UART2_Read();
                
                if (!duplexTimeoutFlag) return dupData;
                else return 0;
                
            case GPSUnit: 
                
                // Reset Timeout
                gpsTimeoutFlag = false;
                
                uint8_t gpsData = UART1_Read();
                
                if (!gpsTimeoutFlag) return gpsData;
                else return 0;
                
            case DiagUnit: return UART4_Read();
            default: return 0xFF;
        }
    } return 0;
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

void SendData(uint8_t *queue, int queueLength, TransmissionUnit unit) {
    
    bool isError = false;
    
    isSending = true;
    
    int packets = 0;
    const int maxPackets = 4;
    while (queueLength > 0 && packets++ < maxPackets) {
//    for (packets = 0; packets < maxPackets; packets++) {
        // System Header Parser
        uint8_t sysID = queue[transmitQueueStartIndex];
        uint8_t timeH = queue[(transmitQueueStartIndex + 1)%QUEUE_SIZE];
        uint8_t timeL = queue[(transmitQueueStartIndex + 2)%QUEUE_SIZE];
        uint16_t dataLength = (queue[(transmitQueueStartIndex + 3)%QUEUE_SIZE]); // << 8) | queue[(transmitQueueStartIndex + 4)%QUEUE_SIZE];
        
        if (dataLength > QUEUE_SIZE) {
            
            // Error : Reset Queue
            isError = true;
            
            ClearQueue(transmitQueue, QUEUE_SIZE, 0);
            
            transmitQueueStartIndex = 0;
            transmitQueueLength = 0;
            queueLength = 0;
            dataLength = 0;
        }
        
        uint8_t headerByte1 = sysID + timeH;
        uint8_t headerByte2 = timeL;
        
        if (!isError) {
            
            switch (unit) {
                case SimplexUnit: TransmitInstrumentDataToSimplex(queue, headerByte1, headerByte2, dataLength);
                    break;
                case DuplexUnit: TransmitInstrumentDataToDuplex(queue, headerByte1, headerByte2, dataLength);
                    break;
                case SimplexOrDuplex:
                    if (!TransmitInstrumentDataToDuplex(queue, headerByte1, headerByte2, dataLength)) {
                        TransmitInstrumentDataToSimplex(queue, headerByte1, headerByte2, dataLength); }
                    break;
                default:
                    break;
            }

            queueLength = max_s(queueLength - HEADER_SIZE - dataLength, 0);
        }
    }   
    
    isSending = false;
}

void TransmitInstrumentDataToSimplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength) {
    
    // Update Queue position properties
    ClearQueue(transmitQueue, HEADER_SIZE, transmitQueueStartIndex);
    transmitQueueStartIndex = (transmitQueueStartIndex + HEADER_SIZE) % QUEUE_SIZE;
    transmitQueueLength = max_s(transmitQueueLength - HEADER_SIZE, 0);

    // Send data in packages
    int i = 0;
    while (i < dataLength) {

        while (IsLineBusy());

        // One package transmission
        int j;
        uint16_t packageLength;

        TransmitPreamble(simplexPackagePreamble);

        Send(headerByte1, SimplexUnit);
        Send(headerByte2, SimplexUnit);
        
        // Package is shortened to make room for header
        packageLength = GetTransmissionPackageLength(SimplexUnit) - 2;

        // Sends enough data for a package
        for (j = 0; j < packageLength; j++) {

            // If package space exceeds instrument's data, fill with 0
            if (j >= dataLength - i) Send(0, SimplexUnit);
                // Send next data byte 
            else Send(queue[(transmitQueueStartIndex + j) % QUEUE_SIZE], SimplexUnit);

        }

        if (ReadACKForUnit(SimplexUnit)) {
            ClearQueue(transmitQueue, min(packageLength, dataLength - i), transmitQueueStartIndex);
            transmitQueueStartIndex = (transmitQueueStartIndex + min(j, dataLength - i)) % QUEUE_SIZE;
            transmitQueueLength = max_s(transmitQueueLength - j, 0);
            i = i + j;
        }

    }
}

bool TransmitInstrumentDataToDuplex(uint8_t *queue, uint8_t headerByte1, uint8_t headerByte2, uint16_t dataLength) {

    int i;

    while (IsLineBusy());

    PollDuplex(duplexDownlinkFilePoll, 0);

    if (ReadACKForUnit(DuplexUnit)) {

        // One package transmission
        uint8_t duplexFormattedSendString[1000] = { NULL };
        char duplexFileHeader[50] = { NULL };
        int duplexFileSize = dataLength + 5;
        
        uint16_t duplexFileLength = 4 + // 4 byte uint
                               strlen(duplexDownlinkFileReady) + // duplex poll code
                               CreateFileHeader(duplexFileHeader,headerByte1 & 0xF0, duplexFileSize) + // file header
                               duplexFileSize + // file size
                               2; // crc-16 word
        if (duplexFileLength % 2 != 0) duplexFileLength += 1;
        
        int formatIndex = 0;
        // 4-byte File Length
        duplexFormattedSendString[formatIndex++] = 0;                           
        duplexFormattedSendString[formatIndex++] = 0;
        duplexFormattedSendString[formatIndex++] = duplexFileLength >> 8;
        duplexFormattedSendString[formatIndex++] = duplexFileLength & 0xFF;
        
        // Duplex Poll Code
        formatIndex += CopyBytes(duplexDownlinkFileReady, duplexFormattedSendString, 0, formatIndex, 5);
        
        // Duplex ESN, File name length, File size, File name
        formatIndex += CopyBytes(duplexFileHeader, duplexFormattedSendString, 0, formatIndex, strlen(duplexFileHeader));
        
        // File Data
        formatIndex += CopyBytes(queue, duplexFormattedSendString, transmitQueueStartIndex, formatIndex, duplexFileSize);
        
        // Make file an even number of bytes
        if (formatIndex % 2 != 0) duplexFormattedSendString[formatIndex++] = 0;
        
        // Get the CRC-16 code for the sent string
        uint16_t crc16Code = CRC16SendBytes(duplexFormattedSendString, formatIndex, 0);
        
        // Send the two sync bytes
        Send(duplexSyncBytes[0], DuplexUnit);
        Send(duplexSyncBytes[1], DuplexUnit);
        
        // Sends enough data for a package
        for (i = 0; i < formatIndex; i++) {

            // Send next data byte
            Send(duplexFormattedSendString[i], DuplexUnit);
        }
        
        // Send the trailing CRC-16 Code
        Send((crc16Code >> 8), DuplexUnit);
        Send(crc16Code & 0x00FF, DuplexUnit);
                
        if (ReadACKForUnit(DuplexUnit)) {
            ClearQueue(transmitQueue, duplexFileSize, transmitQueueStartIndex);
            transmitQueueStartIndex = (transmitQueueStartIndex + duplexFileSize) % QUEUE_SIZE;
            transmitQueueLength = max(transmitQueueLength - duplexFileSize, 0);
        }
    }

    return true;
}

/***************************************
  Transmission Unit Interfacing Methods
 ***************************************/

bool ReadACKForUnit(TransmissionUnit unit) {
    
    int i;
    bool isACK = true;
    
    if (!IS_DEBUG) {
        switch (unit) {
            case SimplexUnit:
                
                while (Read(unit) != 0xAA ) { if (simplexTimeoutFlag) return true; }
                while (Read(unit) != 0x05) { if (simplexTimeoutFlag) return true; }
                
                isACK = (Read(unit) == 0);
                break;
                
            case DuplexUnit:
                
                while (Read(unit) != 0x47) { if (duplexTimeoutFlag) return false; }
                while (Read(unit) != 0x55) { if (duplexTimeoutFlag) return false; }
                
                for (i = 0; i < DUP_RES_LENGTH - DUPLEX_SYNC_LENGTH; i++) {
                    switch (i) {
                        case 4: 
                            isACK = (Read(unit) == 6);
                            break;
                        default: 
                            Read(unit);
                            break;
                    }
                }
                
                if (duplexTimeoutFlag) return false;
                break;
                
                
            default: break;
        }
    }
        
    return isACK;
}

bool IsLineBusy() {
    return (_RD10 > 0);
}

/****************************
  Duplex Interfacing Methods
 ****************************/

size_t CreateFileHeader(char *formattedString, uint8_t instrument, uint16_t dataLength) {
    
    char esn[] = "0-453221";
    char fileName[20] = { NULL };

    // Parse filename from instrument byte
    switch (instrument) {
        case 0x10:             
            sprintf(fileName,"%s%i.dat",langmuirProbeFileName,langmuirProbeFileCount++);
            break;
        case 0x20:             
            sprintf(fileName,"%s%i.dat",magnetometerFileName,magnetometerFileCount++);
            break;
        case 0x30:             
            sprintf(fileName,"%s%i.dat",temperatureFileName,temperatureFileCount++);
            break;
        case 0x40:            
            sprintf(fileName,"%s%i.dat",gpsFileName,gpsFileCount++);
            break;
        case 0x50:
            sprintf(fileName,"%s%i.dat",housekeepingFileName,housekeepingFileCount++);
        default:
            strcpy(fileName, "Data0000.dat");
            break;     
    }
    
    uint8_t nameLen = strlen(fileName);
        
    return sprintf(formattedString,"%s%03i%06i%s",esn,nameLen,dataLength,fileName); 
}

uint16_t GetWaitingFilesCount() {
    
    PollDuplex(duplexWaitingFilesPoll, 0);
    
    if (ReadACKForUnit(DuplexUnit)) {
        
        while (Read(DuplexUnit) != 0x47) { if (duplexTimeoutFlag) { return 0xFF; } }
        while (Read(DuplexUnit) != 0x55) { if (duplexTimeoutFlag) { return 0xFF; } }
        
        int i;
        int responseLength = 13;
        
        uint8_t highByte, lowByte;
        
        for (i = 0; i < responseLength; i++) {
            
            switch (i) {
                case 11: highByte = Read(DuplexUnit); break;
                case 12: lowByte = Read(DuplexUnit); break;
                default: Read(DuplexUnit); break;
            }
        }
        
        return (highByte << 8) | lowByte;
    }
    
    return -1;
}

void SetTotalTime() {
    
    if (isDuplexOn()) {
        
        // Change to a Do-While (!ReadACKForUnit(Duplex))
        bool isAck = false;
        
        do {
            PollDuplex(duplexHousekeepingFilePoll, 0);
            isAck = ReadACKForUnit(DuplexUnit);
        } while (duplexTimeoutFlag);

        if (!isAck) return;
        
        while (Read(DuplexUnit) != 0x47) { if (duplexTimeoutFlag) { return; } }
        while (Read(DuplexUnit) != 0x55) { if (duplexTimeoutFlag) { return; } }

        int i;
        int responseLength = 13;

        char epoch[4] = {NULL};
        time_t totalEpoch;

        for (i = 0; i < responseLength; i++) {
            switch (i) {
                case 10: epoch[0] = Read(DuplexUnit);
                    break;
                case 11: epoch[1] = Read(DuplexUnit);
                    break;
                case 12: epoch[2] = Read(DuplexUnit);
                    break;
                case 13: epoch[3] = Read(DuplexUnit);
                    break;
                default: Read(DuplexUnit);
                    break;
            }
            

//            totalEpoch = (epoch[0] << 24) | (epoch[1] << 16) | (epoch[2] << 8) | (epoch[3]);
            totalEpoch = (epoch[0] * 100 + epoch[1] * 10 + epoch[2] * 1 + epoch[3]);
            totalTime = (time_t)((double)(totalEpoch - DUPLEX_EPOCH_OFFSET) / 3.0 * 24 * 60);
        }
    }
}

void HandleCommand() {

    // Need to make sure that the command string is a set number of bytes
    int i, messageLength;
    char smsHeader[] = "SMS";
    bool isError = false;
    
    if (isDuplexOn() && !isSending) {

        PollDuplex(duplexUplinkSMSPoll, 0);

        if (ReadACKForUnit(DuplexUnit)) {
            
//            char string[67];
//            for (i = 0; i < 67; i++) string[i] = Read(DuplexUnit);
//            while (1);
//            return;
            
            // Read Duplex Sync Bytes & Header
            while (Read(DuplexUnit) != 0x47) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != 0x55) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != 0x00) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != 0x00) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != 0x00) if (duplexTimeoutFlag) { return; }

            // Calculate SMS message length
            // 4: 4 byte message length
            // 5: Duplex Poll
            // 8: 8 byte ESN
            // 9: 9 byte COMM Header
            // 3: SMS Header
            // 14: SMS Message Length Byte Count
            // 5: UNITE Message Header
            // 2: CRC-16 Code
            messageLength = Read(DuplexUnit) - 4 - 5 - 8 - 9 - 3 - 14 - 5;
            
            // Read Duplex Poll Code
            while (Read(DuplexUnit) != duplexUplinkSMSReady[0]) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != duplexUplinkSMSReady[1]) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != duplexUplinkSMSReady[2]) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != duplexUplinkSMSReady[3]) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != duplexUplinkSMSReady[4]) if (duplexTimeoutFlag) { return; }
            
            // ESN & COMM Header
            for (i = 0; i < 17; i++) Read(DuplexUnit);
            
            // Read SMS Header
            while (Read(DuplexUnit) != smsHeader[0]) if (duplexTimeoutFlag) { return; }
            while (Read(DuplexUnit) != smsHeader[1]) if (duplexTimeoutFlag) { return; } 
            while (Read(DuplexUnit) != smsHeader[2]) if (duplexTimeoutFlag) { return; }
            
            // Read SMS Header Message Length
            for (i = 0; i < 14; i++) Read(DuplexUnit);

            // Read UNITE Message Header else throw away
            if (Read(DuplexUnit) != 'U') isError = true;
            if (Read(DuplexUnit) != 'N') isError = true;
            if (Read(DuplexUnit) != 'I') isError = true;
            if (Read(DuplexUnit) != 'T') isError = true;
            if (Read(DuplexUnit) != 'E') isError = true;

            // Read actual command
            Clear(commandString, messageLength, 0);
            
            for (i = 0; i < messageLength; i++) {

                char asciiNum = Read(DuplexUnit);
                commandString[i] = asciiNum & 0x0F;
            }
            
            PollDuplex(duplexCmdReceived, 0);
            
            if (!isError) PerformCommands(commandString, messageLength);
            
        }
    }
    
    return;
}   

/*************************
   Power Manager Methods
 *************************/

void TogglePowerSwitches() {
           
    while(IsLineBusy());
    if (!isSending && !IS_DEBUG) {
        
        TransmitPreamble(powerPackagePreamble);
                
        uint8_t powerPackage[10] = {0x01, commandBoardPowerSwitch, 
                                    0x02, temperaturePowerSwitch, 
                                    0x03, langmuirMagPowerSwitch,
                                    0x04, gpsPowerSwitch,
                                    0x05, duplexPowerSwitch};
                
        int i;
        for (i = 0; i < 35; i++) {
            if (i < 10) Send(powerPackage[i], SimplexUnit);
            else Send(0, SimplexUnit); 
        }
        
        ReadPowerSwitches();
        
        
    } else {
        
    }
}

void ReadPowerSwitches() {
         
    while(Read(SimplexUnit) != 'P') { if (simplexTimeoutFlag) return; }
    while(Read(SimplexUnit) != 'P') { if (simplexTimeoutFlag) return; }
    while(Read(SimplexUnit) != 'P') { if (simplexTimeoutFlag) return; }
    while(Read(SimplexUnit) != 11) { if (simplexTimeoutFlag) return; }
    
//    if (!IsLineBusy(SimplexUnit)) {
        int i;
        for (i = 0; i < POWER_ECHO_LENGTH; i++) {

            switch (i) {
                case 3: temperaturePowerSwitch = Read(SimplexUnit); break;
                case 5: langmuirMagPowerSwitch = Read(SimplexUnit); break;
                case 7: gpsPowerSwitch = Read(SimplexUnit); break;
                case 9: duplexPowerSwitch = Read(SimplexUnit); break;
                case 10: b1Charge = Read(SimplexUnit); b1Charge <<= 8; break;   // Battery 1 Charge High
                case 11: b1Charge += Read(SimplexUnit); break;                  // Battery 1 Charge Low
                case 12: b2Charge = Read(SimplexUnit); b2Charge <<= 8; break;   // Battery 2 Charge High
                case 13: b2Charge += Read(SimplexUnit); break;                  // Battery 2 Charge Low
                case 14: b1Voltage = Read(SimplexUnit); b1Voltage <<= 8; break; // Battery 1 Voltage High
                case 15: b1Voltage += Read(SimplexUnit); break;                 // Battery 1 Voltage Low
                case 16: b2Voltage = Read(SimplexUnit); b2Voltage <<= 8; break; // Battery 2 Voltage High
                case 17: b2Voltage += Read(SimplexUnit); break;                 // Battery 2 Voltage Low
                case 18: b1Current = Read(SimplexUnit); b1Current <<= 8; break; // Battery 1 Current High
                case 19: b1Current += Read(SimplexUnit); break;                 // Battery 1 Current Low
                case 20: b2Current = Read(SimplexUnit); b2Current <<= 8; break; // Battery 2 Current High
                case 21: b2Current += Read(SimplexUnit); break;                 // Battery 2 Current Low
                case 22: bussPlusVoltage = Read(SimplexUnit); bussPlusVoltage <<= 8; break;   // Buss+ Voltage High
                case 23: bussPlusVoltage += Read(SimplexUnit); break;           // Buss+ Voltage Low
                case 24: solar1Voltage = Read(SimplexUnit); solar1Voltage <<= 8; break;     // Solar Panel 1 Voltage High
                case 25: solar1Voltage += Read(SimplexUnit); break;             // Solar Panel 1 Voltage Low 
                case 26: solar2Voltage = Read(SimplexUnit); solar2Voltage <<= 8; break;     // Solar Panel 2 Voltage High
                case 27: solar2Voltage += Read(SimplexUnit); break;             // Solar Panel 2 Voltage Low
                case 28: solar3Voltage = Read(SimplexUnit); solar3Voltage <<= 8; break;     // Solar Panel 3 Voltage High
                case 29: solar3Voltage += Read(SimplexUnit); break;             // Solar Panel 3 Voltage Low
                case 30: solar4Voltage = Read(SimplexUnit); solar4Voltage <<= 8; break;     // Solar Panel 4 Voltage High
                case 31: solar4Voltage += Read(SimplexUnit); break;             // Solar Panel 4 Voltage Low
                case 32: simplexTemp = Read(SimplexUnit); simplexTemp <<= 8; break;       // Simplex Temp High
                case 33: simplexTemp += Read(SimplexUnit); break;               // Simplex Temp Low
                case 34: duplexTemp = Read(SimplexUnit); duplexTemp <<= 8; break;        // Duplex Temp High
                case 35: duplexTemp += Read(SimplexUnit); break;                // Duplex Temp Low
                case 36: epsTemp = Read(SimplexUnit); epsTemp <<= 8; break;           // EPS Temp High
                case 37: epsTemp += Read(SimplexUnit); break;                   // EPS Temp Low
                case 38: isDuplexConnected = Read(SimplexUnit) > 0; break;
                default: Read(SimplexUnit); break;
            }
        }
//    }
}

void SetLangmuirProbePower(bool on) {
    if (on) langmuirMagPowerSwitch = 0xFF;
    else langmuirMagPowerSwitch = 0x00;
}

void SetMagnetometerPower(bool on) {
    SetLangmuirProbePower(on);
}

void SetTemperaturePower(bool on) {
    if (on) temperaturePowerSwitch = 0xFF;
    else temperaturePowerSwitch = 0x00;
}

void SetGPSPower(bool on) {
    if (on) gpsPowerSwitch = 0xFF;
    else gpsPowerSwitch = 0x00;
}

void SetDuplexPower(bool on) {
    if (on) duplexPowerSwitch = 0xFF;
    else duplexPowerSwitch = 0x00;
    
    TogglePowerSwitches();
}