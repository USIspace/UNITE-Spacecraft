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

// Value properties
const int VALUE_ARRAY_SIZE = 2;
uint8_t value[2];
int nextValueIndex = 0;


bool isEndOfMessage = false;

/*****************
  Private Methods
 *****************/

void EndMessage();
void ParseByte(uint8_t, CommandByteIndex);

void addValueByte(uint8_t);
uint16_t convertHexToDecimal(uint8_t *);
unsigned long convertTime(Unit, Unit);
void RunCommand(System, Mode, Property, Unit, uint16_t);

/************************
  Main Command Functions
 ************************/

void PerformCommands(uint8_t *commandString, uint16_t CMD_Length) {

    int i = 0;
    while (i < CMD_Length) {
        while (!isEndOfMessage) {
            ParseByte(commandString[i++], byteIndex++);
        }

        if (system != Break) {
            uint16_t convertedValue = (value[0] << 8) | value[1];
            RunCommand(system, mode, property, unit, convertedValue);
        }
    }
}

void EndMessage() {
    byteIndex = 0;
    nextValueIndex = 0;
    isEndOfMessage = true;
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
            // Transmission doesn't have a unit byte
            if (system == Transmission) ParseByte(byte, byteIndex++); // Skip unit byte if command is for tranmission
            else unit = byte;
            break;
        case OverrideFlag:
            isOverride = byte > 0;
            break;
        case ResetFlag:
            isReset = byte > 0;
            if (system == Transmission) EndMessage();
            break;
        default:
            addValueByte(byte);
            break;
    }
}

void RunCommand(System system, Mode mode, Property property, Unit unit, uint16_t value) {

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
        case LP: instrument = &LangmuirProbe;
            isInstrument = true;
            break;
        case TMP: instrument = &TemperatureSensors;
            isInstrument = true;
            break;
        case MAG: instrument = &Magnetometer;
            isInstrument = true;
            break;
        case CDH: break;
        case EPS: break;
        case GPS: instrument = &GPSUnit;
            isInstrument = true;
            break;
        case Housekeeping: break;
        case Beacon: break;
        default: break;
    }

    // Choose Satellite Mode
    switch (mode) {
        case Interim:

            if (isInstrument) modeProperties = &instrument->Interim;
            else if (isTransmission) unitForMode = &transMode->interim;
            break;
        case Science:

            if (isInstrument) modeProperties = &instrument->Science;
            else if (isTransmission) unitForMode = &transMode->science;
            break;
        case ReEntry:

            if (isInstrument) modeProperties = &instrument->ReEntry;
            else if (isTransmission) unitForMode = &transMode->reentry;
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
                modeProperties->sampleRate = value * convertTime(unit, Min);
                modeProperties1->sampleRate = value * convertTime(unit, Min);
                modeProperties2->sampleRate = value * convertTime(unit, Min);
            } else {
                
                // Update wait timers so that the new wait time occurs only once
                int changeInWait = modeProperties->sampleRate - (value * convertTime(unit, Min));
                switch (system) {
                    case LP: currentLangmuirProbeWait = currentLangmuirProbeWait + changeInWait;
                        break;
                    case MAG: currentMagnetometerWait = currentMagnetometerWait + changeInWait;
                        break;
                    case TMP: currentTemperatureWait = currentTemperatureWait + changeInWait;
                        break;
                    case GPS: currentGPSWait = currentGPSWait + changeInWait;
                        break;
                    default: break;
                }
            }
            break;
        case SweepRate:
            
            switch (system) {
                case LP: unitConversion = convertTime(unit, MilSec) / 5;
                    LP_BUFFER_SIZE = (modeProperties->sweepDuration * convertTime(Sec, MilSec))/(value * unitConversion * 5);
                    break;
                case MAG: unitConversion = convertTime(unit, MilSec) / 100; 
                    MAG_BUFFER_SIZE = (modeProperties->sweepDuration * convertTime(Sec, MilSec))/(value * unitConversion * 100);
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
                    case LP: langmuirProbeCallbackCount = langmuirProbeCallbackCount + changeInRate;
                        break;
                    case MAG: magnetometerCallbackCount = magnetometerCallbackCount + changeInRate;
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
                    case LP: currentLangmuirProbeSweepProgress = currentLangmuirProbeSweepProgress + changeInDuration;
                        break;
                    case MAG: currentMagnetometerSweepProgress = currentMagnetometerSweepProgress + changeInDuration;
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

}

/****************
  Helper Methods
 ****************/

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