#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "SatelliteMode.h"
#include "SampleManager.h"
#include "CommandParser.h"
#include "SystemConfiguration.h"

uint16_t GetSampleRate(Instrument instrument, UNITEMode currentMode) {
    switch (currentMode) {
        case interim: return instrument.Interim.sampleRate;
        case science: return instrument.Science.sampleRate;
        case reentry: return instrument.ReEntry.sampleRate;
        default: return 0;
    }
}
uint16_t GetSweepRate(Instrument instrument, UNITEMode currentMode) {
    switch (currentMode) {
        case interim: return instrument.Interim.sweepRate;
        case science: return instrument.Science.sweepRate;
        case reentry: return instrument.ReEntry.sweepRate;
        default: return 0;
    }
}
uint16_t GetSweepDuration(Instrument instrument, UNITEMode currentMode) {
    switch (currentMode) {
        case interim: return instrument.Interim.sweepDuration;
        case science: return instrument.Science.sweepDuration;
        case reentry: return instrument.ReEntry.sweepDuration;
        default: return 0;
    }
}

Instrument LangmuirProbe = {
    
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};

Instrument TemperatureSensors = {
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};

Instrument Magnetometer = {
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};

Instrument GPSUnit = {
    { 120, 0, 0 },
    { 30, 0, 0 },
    { 15, 0, 0 }
};
