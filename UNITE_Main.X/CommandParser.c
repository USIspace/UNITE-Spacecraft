#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"

CommandByteIndex byteIndex = 0;

System system;
Mode mode;
Property property;
Unit unit;
bool isOverride = false;
bool isReset = false;

// Command Sequence #
uint8_t seqByte1 = 0;
uint8_t seqByte2 = 0;
uint8_t seqByte3 = 0;

// Value properties
const int VALUE_ARRAY_SIZE = 4;
uint8_t value[4];
int nextValueIndex = 0;

// Boolean on/off for end of command
bool isEndOfCommand = false;

/*****************
  Private Methods
 *****************/

void EndMessage();
void ParseByte(uint8_t, CommandByteIndex);

bool IsNextCommand(uint8_t *sequenceNumber, int seqLength);
void addValueByte(uint8_t);
uint16_t convertHexToDecimal(uint8_t *);
unsigned long convertTime(Unit, Unit);
void RunCommand(System, Mode, Property, Unit, unsigned long);

/************************
  Main Command Functions
 ************************/

void PerformCommands(uint8_t *commandString, uint16_t CMD_Length) {

    int i = 0;
    while (i < CMD_Length) {
        
        // Sequence Checking Algorithm
        if (IsNextCommand(commandString, 3)) {
            
            i += 3;
            
            while (!isEndOfCommand && i < CMD_Length) {
                ParseByte(commandString[i++], byteIndex++);
                
            }

            if (system != Break) {

                // Convert value array into 2 byte unsigned integer
                uint16_t convertedValue = (value[0] * 1000) + (value[1] * 100) + (value[2] * 10) + value[3];

                // Run Command
                RunCommand(system, mode, property, unit, convertedValue);
                
                // Reset end of command boolean
                isEndOfCommand = false;
            }
            
        } else {
            
            i = CMD_Length;
        } 
    }
    
    return;
}

void EndMessage() {
    byteIndex = 0;
    nextValueIndex = 0;
    isEndOfCommand = true;
}

void ParseByte(uint8_t byte, CommandByteIndex index) {

    switch (index) {
        case SystemFlag:
            system = byte;
            if (system == Break) EndMessage();
            break;
        case ModeFlag:
            mode = byte;
            break;
        case PropertyFlag:
            property = byte;
            break;
        case UnitFlag:
            unit = byte;
            break;
        case OverrideFlag:
            isOverride = byte > 0;
            break;
        case ResetFlag:
            isReset = byte > 0;
            break;
        default:
            addValueByte(byte);
            break;
    }
}

void RunCommand(System system, Mode mode, Property property, Unit unit, unsigned long value) {

    bool isInstrument = false;
    Instrument *instrument;
    Properties *modeProperties;
    Properties *modeProperties1;
    Properties *modeProperties2;

    bool isTransmission = false;
    TransmissionMode *transMode;
    TransmissionUnit *unitForMode;
    TransmissionUnit *unitForMode1;
    TransmissionUnit *unitForMode2;


    // Choose System
    switch (system) {
        case Transmission: transMode = &TransmissionUnitForMode;
            isTransmission = true;
            break;
        case LPSubSys: instrument = &LangmuirProbe;
            isInstrument = true;
            break;
        case TMPSubSys: instrument = &TemperatureSensors;
            isInstrument = true;
            break;
        case MAGSubSys: instrument = &Magnetometer;
            isInstrument = true;
            break;
        case CDHSubSys: break;
        case EPSSubSys: break;
        case GPSSubSys: instrument = &GPS;
            isInstrument = true;
            break;
        default: break;
    }

    // Choose Satellite Mode
    switch (mode) {
        case FirstWeek:
            
            if (isInstrument) modeProperties = &instrument->FirstWeek;
            else if (isTransmission) unitForMode = &transMode->firstWeek;
        case Interim:

            if (isInstrument) modeProperties = &instrument->Interim;
            else if (isTransmission) unitForMode = &transMode->interim;
            break;
            
        case Stabilize:
            
            if (isInstrument) modeProperties = &instrument->Stabilize;
            else if (isTransmission) unitForMode = &transMode->stabilize;
            break;
            
        case Science:

            if (isInstrument) modeProperties = &instrument->Science;
            else if (isTransmission) unitForMode = &transMode->science;
            break;
        case ReEntry:

            if (isInstrument) modeProperties = &instrument->ReEntry;
            else if (isTransmission) unitForMode = &transMode->reentry;
            break;
            
        case Fallback:
            
            if (isInstrument) modeProperties = &instrument->Fallback;
            else if (isTransmission) unitForMode = &transMode->fallback;
            break;
            
        case Global:

            if (isInstrument) {
                modeProperties = &instrument->Interim;
                modeProperties1 = &instrument->Science;
                modeProperties2 = &instrument-> ReEntry;
            }
            else if (isTransmission) {
                unitForMode = &transMode->interim;
                unitForMode1 = &transMode->science;
                unitForMode2 = &transMode->reentry;
            }
            break;
        default: break;
    }

    // Convert Value to proper ticker
    long unitConversion = 1;
    switch (property) {
        case SamplingRate:

            if (isOverride) {
                if (unit == Sample) {
                    if (system == LPSubSys && isReset) lpSamplesPerCalibration = value;
                } else {
                    modeProperties->sampleRate = value * convertTime(unit, Min);
                    modeProperties1->sampleRate = value * convertTime(unit, Min);
                    modeProperties2->sampleRate = value * convertTime(unit, Min);
                }
            } else {
                
                // Update wait timers so that the new wait time occurs only once
                int changeInWait = unit == Sample ? value : modeProperties->sampleRate - (value * convertTime(unit, Min));
                switch (system) {
                    case LPSubSys: 
                        if (isReset) currentLangmuirProbeCalWait += changeInWait;
                        else currentLangmuirProbeWait += changeInWait;
                        break;
                    case MAGSubSys: currentMagnetometerWait += changeInWait;
                        break;
                    case TMPSubSys: currentTemperatureWait += changeInWait;
                        break;
                    case GPSSubSys: currentGPSWait += changeInWait;
                        break;
                    default: break;
                }
            }
            break;
        case SweepRate:
            
            switch (system) {
                case LPSubSys: unitConversion = convertTime(unit, MilSec) / 5;
//                    LP_BUFFER_SIZE = (modeProperties->sweepDuration * convertTime(Sec, MilSec))/(value * unitConversion * 5);
                    break;
                case MAGSubSys: unitConversion = convertTime(unit, Sec); 
                    break;
                default: break;
            }

            if (isOverride) {
                modeProperties->sweepRate = value * unitConversion;
                modeProperties1->sweepRate = value * unitConversion;
                modeProperties2->sweepRate = value * unitConversion;
            } else {
                
                int changeInRate = modeProperties->sweepRate - (value * unitConversion);
                switch (system) {
                    case LPSubSys: langmuirProbeCallbackCount = langmuirProbeCallbackCount + changeInRate;
                        break;
                    case MAGSubSys: magnetometerCallbackCount = magnetometerCallbackCount + changeInRate;
                        break;
                    default: break;
                }
            }

            break;
        case SweepDuration:

            if (isOverride) {
                modeProperties->sweepDuration = value * convertTime(unit, Sec);
                modeProperties1->sweepDuration = value * convertTime(unit, Sec);
                modeProperties2->sweepDuration = value * convertTime(unit, Sec);
            } else {
                
                int changeInDuration = modeProperties->sweepDuration - (value * convertTime(unit, Sec));
                switch (system) {
                    case LPSubSys: currentLangmuirProbeSampleProgress = currentLangmuirProbeSampleProgress + changeInDuration;
                        break;
                    case MAGSubSys: currentMagnetometerSweepProgress = currentMagnetometerSweepProgress + changeInDuration;
                        break;
                    default: break;
                }
            }
            break;
        case Simplex:
            *unitForMode = SimplexUnit;
            *unitForMode1 = SimplexUnit;
            *unitForMode2 = SimplexUnit;

            break;
        case Duplex:
            *unitForMode = DuplexUnit;
            *unitForMode1 = DuplexUnit;
            *unitForMode2 = DuplexUnit;

            break;
        case Simplex_Duplex:
            *unitForMode = SimplexOrDuplex;
            *unitForMode1 = SimplexOrDuplex;
            *unitForMode2 = SimplexOrDuplex;

            break;
        default:
            break;
    }

    return;
}

/****************
  Helper Methods
 ****************/

bool IsNextCommand(uint8_t *sequenceNumber, int seqLength) {
    
    int i;
    for (i = 0; i < seqLength; i++) {
        switch (i) {
            case 0:
                if (sequenceNumber[i] > seqByte1) {
                    seqByte1 = sequenceNumber[i];
                    i += 3;
                }
                else if (sequenceNumber[i] == seqByte1);
                else return false;
                break;
            case 1:
                if (sequenceNumber[i] > seqByte2) {
                    seqByte2 = sequenceNumber[i];
                    i += 2;
                } else if (sequenceNumber[i] == seqByte2);
                else return false;
                break;
            case 2:
                if (sequenceNumber[i] > seqByte3) {
                    seqByte3 = sequenceNumber[i];
                    i++;
                } else return false;
                break;
            default:
                break;
        }
    }
    
    return true;
}

void addValueByte(uint8_t byte) {

    value[nextValueIndex++] = byte;
    if (nextValueIndex >= VALUE_ARRAY_SIZE) EndMessage();
}

uint16_t convertHexToDecimal(uint8_t *bits) {

    uint16_t decimalValue = 0;
    int i;
    for (i = 0; i < VALUE_ARRAY_SIZE; i++) {
        decimalValue = decimalValue + bits[0] * Pow(16, VALUE_ARRAY_SIZE - i);
    }

    return decimalValue;
}

unsigned long convertTime(Unit current, Unit final) {

    // Make sure the final unit is smaller than the original
    if (current > final) {
        switch (current) {
            case Day: return convertTime(--current, final) * 24;
                break;
            case Hour: return convertTime(--current, final) * 60;
                break;
            case Min: return convertTime(--current, final) * 60;
                break;
            case Sec: return convertTime(--current, final) * 1000;
                break;
            default: return 1;
                break;
        }
    } else {
        return 1;
    }
}