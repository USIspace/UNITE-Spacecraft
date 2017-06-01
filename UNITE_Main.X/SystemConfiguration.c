#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"

TransmissionUnit currentTransmissionUnit = SimplexOnly;

Instrument LangmuirProbe = {
    
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};

Instrument TemperatureSensors = {
    { 2, 0, 0 },
    { 1, 0, 0 },
    { 1, 0, 0 }
};

Instrument Magnetometer = {
    { 25, 1, 20 },
    { 20, 1, 20 },
    { 15, 1, 20 }
};

Instrument GPSUnit = {
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};

uint16_t GetSampleRate(Instrument *instrument) {
    
    switch (currentMode) {
        case interim: return instrument->Interim.sampleRate;
        case science: return instrument->Science.sampleRate;
        case reentry: return instrument->ReEntry.sampleRate;
        default: return 0;
    }
}
uint16_t GetSweepRate(Instrument *instrument) {
    
    switch (currentMode) {
        case interim: return instrument->Interim.sweepRate;
        case science: return instrument->Science.sweepRate;
        case reentry: return instrument->ReEntry.sweepRate;
        default: return 0;
    }
}
uint16_t GetSweepDuration(Instrument *instrument) {
    
    switch (currentMode) {
        case interim: return instrument->Interim.sweepDuration;
        case science: return instrument->Science.sweepDuration;
        case reentry: return instrument->ReEntry.sweepDuration;
        default: return 0;
    }
}

uint8_t GetSystemHeaderID(System system) {
    switch (system) {
        case LP: return 0x10;
        case MAG: return 0x20;
        case TMP: return 0x30;
        case GPS: return 0x40;
        default: return 0;
    }
}

uint16_t GetDayTimeInMin(unsigned long totalMissionTimeInSec) {
    return (totalMissionTimeInSec / 60) % 1440;
}

uint8_t GetTransmissionPackageLength(TransmissionUnit unit) {
    switch(unit) {
        case SimplexOnly: return 35;
        case DuplexOnly: return 56;
        default: return 0;
    }
}


