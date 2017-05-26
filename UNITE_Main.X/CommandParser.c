#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"

CommandByteIndex byteIndex = 0;

System system;
Mode mode;
Property property;
Unit unit;
bool isOverride = false;
bool isReset = false;

// Value properties
const int VALUE_ARRAY_SIZE = 4;
uint8_t value[4];
int nextValueIndex = 0;


bool isEndOfMessage = false;

void PerformCommands() {
    
    while (!isEndOfMessage) {
        uint8_t cmdByte = GetNextByte();
        ParseByte(cmdByte, byteIndex);
    }
}


void EndMessage() {
    byteIndex = 0;
    nextValueIndex = 0;
    isEndOfMessage = true;
}

uint8_t GetNextByte() {
    return 0;
}


void ParseByte(uint8_t byte, CommandByteIndex index) {
    
    switch (index) {
        case SystemFlag: 
            system = byte;
            byteIndex++;
        case ModeFlag:
            mode = byte;
            byteIndex++;
        case PropertyFlag:
            property = byte;
            byteIndex++;
        case UnitFlag:
            // Transmission doesn't have a unit byte
            if (system == Transmission) {       
                ParseByte(byte, ++byteIndex);   // Skip unit byte if command is for tranmission
            } else {
                unit = byte;
                byteIndex++;
            }
        case OverrideFlag:
            if (byte > 0) { isOverride = true; }
            else { isOverride = false; }
            byteIndex++;
        case ResetFlag:
            if (byte > 0) { isReset = true; }
            else { isReset = false; }
            
            if (system == Transmission) {
                EndMessage();
            } else { 
                byteIndex++;
            }
        default:
            addValueByte(byte);
    }
}

void addValueByte(uint8_t byte) {
    
    value[nextValueIndex++] = byte;
    if (nextValueIndex > VALUE_ARRAY_SIZE) { EndMessage(); }
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
            case Hour: return convertTime(--current, final) * 60;
            case Min: return convertTime(--current, final) * 60;
            case Sec: return convertTime(--current, final) * 1000;
            default: return 1;
        }
    } else {
        return 1;
    }
}

void RunCommand(System system, Mode mode, Property property, Unit unit, unsigned long value) {
    
    Instrument *instrument;
    Properties *modeProperties;
    
    // Choose System
    switch (system) {
        case Transmission: break;
        case LP: instrument = &LangmuirProbe;
        case TMP: instrument = &TemperatureSensors;
        case MAG: instrument = &Magnetometer;
        case CDH: break;
        case EPS: break;
        case GPS: instrument = &GPSUnit;
        case Housekeeping: break;
        case Beacon: break;
        default: break;
    }
    
    // Choose Satellite Mode
    switch (mode) {
        case Interim: modeProperties = &instrument->Interim;
        case Science: modeProperties = &instrument->Science;
        case ReEntry: modeProperties = &instrument->ReEntry;
        case Global: break;
        default: break;
    }
    
    // Convert Value to proper ticker
    if (unit != None) {
        unsigned long unitConversion = 0;
        switch (property) {
            case SamplingRate: 
                
                modeProperties->sampleRate = value * convertTime(unit, Sec);
                
            case SweepRate:
                
                switch (system) {
                    case LP: unitConversion = convertTime(unit, MilSec) / 5;
                    case MAG: unitConversion = convertTime(unit, MilSec) / 100;
                    default: break;
                }
                
                modeProperties->sweepDuration = value * unitConversion;
                
            case SweepDuration:
                
                modeProperties->sampleRate = value * convertTime(unit, Sec);
                
            default:
                break;
        }
    }
}